#pragma once
#include <cstdio>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

template<class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...)->overloaded<Ts...>;

struct EvaluationContext {
    std::map<std::string, bool> predicates;
};

class Expression;
class Statement {
public:
    enum Type { Print, Set, Expr, PrintAtoms, PrintTable };

    Statement();
    Statement(std::shared_ptr<Expression> other, Type type = Type::Print);
    Statement(const std::string &pred, std::shared_ptr<Expression> other);

    virtual ~Statement();

    virtual int  print();
    virtual void exec(EvaluationContext &ec);

protected:
    Type                        type;
    std::string                 pred;
    std::shared_ptr<Expression> other;
};

class Expression : public Statement, public std::enable_shared_from_this<Expression> {
public:
    Expression(std::weak_ptr<Expression> parent)
        : parent(std::move(parent))
    {
    }
    virtual ~Expression() = default;
    void        exec(EvaluationContext &ec) {}
    virtual int eval(EvaluationContext &ec) = 0;
    virtual int print() { return 0; };

    virtual std::list<std::string>                 atoms() const { return {}; };
    virtual std::list<std::shared_ptr<Expression>> childs() const { return {}; };

    void update_parents()
    {
        for (auto &&c : childs()) {
            c->parent = shared_from_this(); // Luke, I am your father ;-;
            c->update_parents();
        }
    }

    virtual std::shared_ptr<Expression> deepcopy() const = 0;

    virtual void visit(class Visitor &v) = 0;

    std::weak_ptr<Expression> parent;
};

class Visitor {
public:
    virtual bool operator()(std::shared_ptr<class BinaryExpression> exp) { return true; }
    virtual bool operator()(std::shared_ptr<class NegExpression> exp) { return true; }
    virtual bool operator()(std::shared_ptr<class ConstantExpression> exp) { return true; }
    virtual bool operator()(std::shared_ptr<class PredExpression> exp) { return true; }
};

template<typename BE, typename NE, typename CE, typename PE>
struct OverloadedVisitor : Visitor {
    template<typename BEs, typename NEs, typename CEs, typename PEs>
    OverloadedVisitor(BEs &&bes, NEs &&nes, CEs &&ces, PEs &&pes)
        : be(std::forward<BEs>(bes))
        , ne(std::forward<NEs>(nes))
        , ce(std::forward<CEs>(ces))
        , pe(std::forward<PEs>(pes))
    {
    }

    bool operator()(std::shared_ptr<class BinaryExpression> exp) override { return be(std::move(exp)); }
    bool operator()(std::shared_ptr<class NegExpression> exp) override { return ne(std::move(exp)); }
    bool operator()(std::shared_ptr<class ConstantExpression> exp) override { return ce(std::move(exp)); }
    bool operator()(std::shared_ptr<class PredExpression> exp) override { return pe(std::move(exp)); }

private:
    BE be;
    NE ne;
    CE ce;
    PE pe;
};

template<class... Ts>
OverloadedVisitor(Ts...)->OverloadedVisitor<Ts...>;

template<typename CRTP>
class CRTPExpression : public Expression {
public:
    template<typename... Args>
    CRTPExpression(Args &&... args)
        : Expression(std::forward<Args>(args)...)
    {
    }

    void visit(Visitor &v)
    {
        const auto recurse = v(std::static_pointer_cast<CRTP>(shared_from_this()));
        if (recurse) {
            for (auto &&c : childs()) {
                c->visit(v);
            }
        }
    }
};

class BinaryExpression : public CRTPExpression<BinaryExpression> {
public:
    enum Type { And, Or, Impl, BiImpl };

    BinaryExpression(std::shared_ptr<Expression> lhs, std::shared_ptr<Expression> rhs, Type op, std::weak_ptr<Expression> parent = {})
        : CRTPExpression(std::move(parent))
        , lhs(std::move(lhs))
        , rhs(std::move(rhs))
        , op(op)
    {
    }

    int eval(EvaluationContext &ec) override
    {
        switch (op) {
        case Type::And:
            return lhs->eval(ec) && rhs->eval(ec);
        case Type::Or:
            return lhs->eval(ec) || rhs->eval(ec);
        case Type::Impl:
            return !lhs->eval(ec) || rhs->eval(ec);
        case Type::BiImpl:
            return lhs->eval(ec) == rhs->eval(ec);
        };
    }

    int print() override
    {
        printf("(");
        const int lc = lhs->print();
        switch (op) {
        case Type::And:
            printf(" ∧ ");
            break;
        case Type::Or:
            printf(" ∨ ");
            break;
        case Type::Impl:
            printf(" → ");
            break;
        case Type::BiImpl:
            printf(" ↔ ");
            break;
        };
        const int rc = rhs->print();
        printf(")");
        return 2 + 3 + lc + rc;
    }

    virtual std::list<std::string> atoms() const override
    {
        auto       ats    = lhs->atoms();
        const auto rhsats = rhs->atoms();
        ats.insert(end(ats), begin(rhsats), end(rhsats));
        return ats;
    }

    virtual std::list<std::shared_ptr<Expression>> childs() const override { return { lhs, rhs }; }

    virtual std::shared_ptr<Expression> deepcopy() const override
    {
        return std::make_shared<BinaryExpression>(lhs->deepcopy(), rhs->deepcopy(), op);
    }

    Type                        op;
    std::shared_ptr<Expression> lhs, rhs;
};

class PredExpression : public CRTPExpression<PredExpression> {
public:
    PredExpression(const std::string &name, std::weak_ptr<Expression> parent = {})
        : CRTPExpression(std::move(parent))
        , name(name)
    {
    }
    int print() override
    {
        printf("%s", name.c_str());
        return name.length();
    }
    int eval(EvaluationContext &ec) override { return ec.predicates[name]; }

    std::list<std::string> atoms() const override { return { name }; }

    virtual std::shared_ptr<Expression> deepcopy() const override { return std::make_shared<PredExpression>(name); }

private:
    std::string name;
};

class ConstantExpression : public CRTPExpression<ConstantExpression> {
public:
    ConstantExpression(bool value, std::weak_ptr<Expression> parent = {})
        : CRTPExpression(std::move(parent))
        , value(value)
    {
    }

    int print() override
    {
        printf("%s", value ? "tt" : "ff");
        return 2;
    }
    int eval(EvaluationContext &) override { return value; }

    std::list<std::string> atoms() const override { return {}; }

    virtual std::shared_ptr<Expression> deepcopy() const override { return std::make_shared<ConstantExpression>(value); }

private:
    bool value;
};

class NegExpression : public CRTPExpression<NegExpression> {
public:
    NegExpression(std::shared_ptr<Expression> other, std::weak_ptr<Expression> parent = {})
        : CRTPExpression(std::move(parent))
        , other(std::move(other))
    {
    }

    int print() override
    {
        printf("(¬");
        const int c = other->print();
        printf(")");

        return 3 + c;
    }

    int eval(EvaluationContext &ec) override { return !other->eval(ec); }

    std::list<std::string>                 atoms() const override { return { other->atoms() }; }
    std::list<std::shared_ptr<Expression>> childs() const override { return { other }; }
    virtual std::shared_ptr<Expression>    deepcopy() const override { return std::make_shared<NegExpression>(other->deepcopy()); }

    std::shared_ptr<Expression> other;
};

class StatementList {
public:
    StatementList() = default;

    StatementList(Statement *stmt) { statements.push_back(stmt); }

    StatementList(StatementList &other, Statement *stmt)
        : statements(other.statements)
    {
        other.statements.clear();
        statements.push_back(stmt);
    }

    StatementList(const StatementList &) = delete;
    StatementList &operator=(const StatementList &) = delete;

    StatementList(StatementList &&) = default;
    StatementList &operator=(StatementList &&) = default;

    void add(Statement *stmt) { statements.push_back(stmt); }

    virtual ~StatementList()
    {
        for (auto stmt : statements) {
            if (stmt)
                delete stmt;
        }
    }

    void run()
    {
        EvaluationContext ec;
        for (auto stmt : statements) {
            if (stmt) {
                stmt->exec(ec);
            }
        }
        printf("\n");
    }

    std::vector<Statement *> statements;
};

class SimplePrintWalker : public Visitor {
    bool operator()(std::shared_ptr<BinaryExpression>) override
    {
        printf("Found binary expression!\n");
        return true;
    }
    bool operator()(std::shared_ptr<NegExpression>) override
    {
        printf("Found neg expression!\n");
        return true;
    }
    bool operator()(std::shared_ptr<ConstantExpression>) override
    {
        printf("Found constant expression!\n");
        return true;
    }
    bool operator()(std::shared_ptr<PredExpression>) override
    {
        printf("Found pred expression!\n");
        return true;
    }
};

namespace nnf {
class Walker : public Visitor {
public:
    bool operator()(std::shared_ptr<BinaryExpression> e) override { return false; }
    bool operator()(std::shared_ptr<NegExpression> e) override { return false; }
    bool operator()(std::shared_ptr<PredExpression> e) override { return false; }
    bool operator()(std::shared_ptr<ConstantExpression> e) override { return false; }
};
}

void make_nnf(std::shared_ptr<Expression> &input);
