#include "ast.h"

Statement::~Statement()
{
    if (other && type != Expr) {
        delete other;
    }
}

void Statement::print()
{
    switch (type) {
    case Type::Print:
        printf("Print: ");
        other->print();
        break;
    case Type::Set:
        printf("Set %s to ", pred.c_str());
        other->print();
        break;
    case Type::PrintAtoms:
        printf("Printing atoms of ");
        other->print();
        break;
    default:
        printf("Unknown statement type");
        break;
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
    default:
        break;
    }
}
