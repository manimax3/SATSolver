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
    default:
        break;
    }
}

void Statement::exec(EvaluationContext &ec)
{
    switch (type) {
    case Type::Print:
        other->print();
        printf(" ==> %s\n", other->eval(ec) ? "tt" : "ff");
        break;
    case Type::Set:
        ec.predicates[pred] = other->eval(ec);
        break;
    default:
        break;
    }
}
