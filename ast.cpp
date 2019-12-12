#include "ast.h"

#include <algorithm>

template<typename T>
auto cartesian(const std::list<std::list<T>> &input)
{
    std::list<std::list<T>> output = { {} };

    for (auto &&u : input) {
        std::list<std::list<T>> r;
        for (auto &&x : output) {
            for (auto &&y : u) {
                r.push_back(x);
                r.back().push_back(y);
            }
        }
        output = std::move(r);
    }

    return output;
}

Statement::Statement()
    : type(Expr)
{
}

Statement::Statement(std::shared_ptr<Expression> other, Type type)
    : type(type)
    , other(std::move(other))
{
    this->other->update_parents();
}

Statement::Statement(const std::string &pred, std::shared_ptr<Expression> other)
    : type(Set)
    , pred(pred)
    , other(std::move(other))
{
    this->other->update_parents();
}

Statement::~Statement() {}

int Statement::print()
{
    switch (type) {
    case Type::Print:
        return printf("Print: ") + other->print();
    case Type::Set:
        return printf("Set %s to ", pred.c_str()) + other->print();
    case Type::PrintAtoms:
        return printf("Printing atoms of ") + other->print();
    default:
        return printf("Unknown statement type");
    }
}

void Statement::exec(EvaluationContext &ec)
{
    switch (type) {
    case Type::Print:
        other->print();
        printf(" ⇒ %s\n", other->eval(ec) ? "tt" : "ff");
        break;
    case Type::Set:
        ec.predicates[pred] = other->eval(ec);
        break;
    case Type::PrintAtoms: {
        const auto atoms = other->atoms();
        printf("Atoms in ");
        other->print();
        printf(" : ");
        for (auto &&a : atoms) {
            printf("%s, ", a.c_str());
        }
        printf("\n");
        break;
    }

    case Type::PrintTable: {
        // 1. Get the atoms
        // 2. for all the combinations print the row
        // Assuming only up to depth 2
        auto atoms = other->atoms();
        atoms.sort();
        atoms.erase(std::unique(begin(atoms), end(atoms)), end(atoms));

        const auto                       rows     = 2 ^ atoms.size();
        const std::list<bool>            elements = { false, true };
        const std::list<std::list<bool>> cpin(atoms.size(), elements);

        const auto cproduct = cartesian(cpin);

        for (auto &&a : atoms) {
            printf(" | %2s", a.c_str());
        }

        const auto     childs = other->childs();
        std::list<int> exprlengths;

        for (auto &&c : childs) {
            printf(" | ");
            const auto count = c->print();
            exprlengths.push_back(count);

            for (int i = 2 - count; i > 0; i--) {
                printf(" ");
            }
        }

        printf(" | ");
        const auto count = other->print();
        exprlengths.push_back(count);

        printf(" |\n");

        for (auto &&ctuple : cproduct) {
            EvaluationContext ec;
            std::size_t       counter = 0;

            // Printing the atoms
            for (auto &&a : ctuple) {
                auto atom = begin(atoms);
                advance(atom, counter++);
                ec.predicates[*atom] = a;

                printf(" | %*s", static_cast<int>(atom->length()), a ? "tt" : "ff");
            }
            counter = 0;

            // 1. level depth
            auto exprlength = begin(exprlengths);
            for (auto &&c : childs) {
                printf(" | %*s", *exprlength, c->eval(ec) ? "tt" : "ff");
                advance(exprlength, 1);
            }

            // 0. level depth
            printf(" | %*s", *exprlength, other->eval(ec) ? "tt" : "ff");

            printf(" |\n");
        }

        break;
    }
    case Type::PrintNNF: {
        make_nnf(other);
        other->print();
        printf(" ⇒ %s\n", other->eval(ec) ? "tt" : "ff");
        break;
    }
    default:
        break;
    }
}

void make_nnf(std::shared_ptr<Expression> &input)
{
    OverloadedVisitor implremover{ [](std::shared_ptr<BinaryExpression> expr) {
                                      if (expr->op == BinaryExpression::Impl) {
                                          expr->op  = BinaryExpression::Or;
                                          auto nota = std::make_shared<NegExpression>(expr->lhs, expr);
                                          expr->lhs = std::move(nota);
                                      } else if (expr->op == BinaryExpression::BiImpl) {
                                          const auto a    = expr->lhs;
                                          const auto b    = expr->rhs;
                                          const auto nota = std::make_shared<NegExpression>(a, expr);

                                          const auto a2   = a->deepcopy();
                                          const auto b2   = b->deepcopy();
                                          const auto notb = std::make_shared<NegExpression>(b2, expr);

                                          expr->lhs = std::make_shared<BinaryExpression>(nota, b, BinaryExpression::Or, expr);
                                          expr->rhs = std::make_shared<BinaryExpression>(notb, a2, BinaryExpression::Or, expr);
                                          expr->op  = BinaryExpression::And;
                                      }
                                      return true;
                                  },
                                   [](auto &&) { return true; }, [](auto &&) { return false; }, [](auto &&) { return false; } };

    OverloadedVisitor nnf{ [](std::shared_ptr<BinaryExpression> expr) {
                              { // clang format you good?
                                  auto a = expr->lhs;
                                  auto b = expr->rhs;
                                  make_nnf(a);
                                  make_nnf(b);
                                  expr->lhs = a;
                                  expr->rhs = b;
                                  return false;
                              }
                          },
                           [&input](std::shared_ptr<NegExpression> expr) {
                               // Double neg
                               if (dynamic_cast<NegExpression *>(expr->childs().front().get())) {
                                   auto e = expr->childs().front()->childs().front();
                                   make_nnf(e);
                                   e->parent = input->parent;
                                   input     = e;
                                   return false; // Maybe false?
                               } else if (auto *bexpr = dynamic_cast<BinaryExpression *>(expr->childs().front().get())) {
                                   if (bexpr->op == BinaryExpression::And) {
                                       const auto b    = bexpr->lhs;
                                       const auto c    = bexpr->rhs;
                                       const auto notb = std::make_shared<NegExpression>(b);
                                       const auto notc = std::make_shared<NegExpression>(c);

                                       auto notbornotc    = std::make_shared<BinaryExpression>(notb, notc, BinaryExpression::Or);
                                       notbornotc->parent = input->parent;

                                       input = notbornotc;
                                       make_nnf(input);
                                   } else if (bexpr->op == BinaryExpression::Or) {
                                       const auto b    = bexpr->lhs;
                                       const auto c    = bexpr->rhs;
                                       const auto notb = std::make_shared<NegExpression>(b);
                                       const auto notc = std::make_shared<NegExpression>(c);

                                       auto notbornotc    = std::make_shared<BinaryExpression>(notb, notc, BinaryExpression::And);
                                       notbornotc->parent = input->parent;

                                       input = notbornotc;
                                       make_nnf(input);
                                   }
                               }

                               return false;
                           },
                           [](auto &&) { return false; }, [](auto &&) { return false; } };

    input->visit(implremover);
    input->visit(nnf);
}
