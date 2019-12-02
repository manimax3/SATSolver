#pragma once
#include <cstdio>
#include <string>

class Expression {
   public:
    virtual ~Expression() = default;
    virtual void print(){};
};

class PredExpression : public Expression {
   public:
    PredExpression(const std::string &name) : name(name) {}
	void print() override {
		printf("%s", name.c_str());
	}

   private:
    std::string name;
};

class ImplExpression : public Expression {
   public:
    ImplExpression(Expression *lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}

    ~ImplExpression() {
	if (lhs) delete lhs;
	if (rhs) delete rhs;
    }
	void print() override {
		printf("(");
		lhs->print();
		printf(" -> ");
		rhs->print();
		printf(")");
	}

   private:
    Expression *lhs = nullptr, *rhs = nullptr;
};

class BiImplExpression : public Expression {
   public:
    BiImplExpression(Expression *lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}

    ~BiImplExpression() {
	if (lhs) delete lhs;
	if (rhs) delete rhs;
    }
	void print() override {
		printf("(");
		lhs->print();
		printf(" <-> ");
		rhs->print();
		printf(")");
	}

   private:
    Expression *lhs = nullptr, *rhs = nullptr;
};

class AndExpression : public Expression {
   public:
    AndExpression(Expression *lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}

    ~AndExpression() {
	if (lhs) delete lhs;
	if (rhs) delete rhs;
    }
	void print() override {
		printf("(");
		lhs->print();
		printf(" and ");
		rhs->print();
		printf(")");
	}

   private:
    Expression *lhs = nullptr, *rhs = nullptr;
};

class OrExpression : public Expression {
   public:
    OrExpression(Expression *lhs, Expression *rhs) : lhs(lhs), rhs(rhs) {}

    ~OrExpression() {
	if (lhs) delete lhs;
	if (rhs) delete rhs;
    }
	void print() override {
		printf("(");
		lhs->print();
		printf(" or ");
		rhs->print();
		printf(")");
	}

   private:
    Expression *lhs = nullptr, *rhs = nullptr;
};

class NegExpression : public Expression {
   public:
    NegExpression(Expression *other) : other(other) {}

    ~NegExpression() {
	if (other) delete other;
    }
	void print() override {
		printf("(¬");
		other->print();
		printf(")");
	}

   private:
    Expression *other = nullptr;
};
