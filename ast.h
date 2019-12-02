#pragma once
#include <cstdio>

class Expression {
   public:
    Expression(Expression *lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}

    ~Expression() {
	if (lhs) delete lhs;
	if (rhs) delete rhs;
    }

   private:
    Expression *lhs, *rhs;
};
