#pragma once
#include <cstdio>
#include <string>
#include <vector>

class Statement {
   public:
    virtual ~Statement() = default;
    virtual void print(){};
};

class SetStatement : public Statement {
   public:
    SetStatement(const std::string &pred, bool value)
	: pred(pred), value(value) {}

    void print() override { printf("Setting %s to %i", pred.c_str(), value); }

   private:
    std::string pred;
    bool value;
};

class Expression : public Statement {
   public:
    virtual ~Expression() = default;
    virtual void print(){};
};

class PredExpression : public Expression {
   public:
    PredExpression(const std::string &name) : name(name) {}
    void print() override { printf("%s", name.c_str()); }

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

class StatementList {
   public:
    StatementList(Statement *stmt) { statements.push_back(stmt); }

    StatementList(StatementList &other, Statement *stmt)
	: statements(other.statements) {
	other.statements.clear();
	statements.push_back(stmt);
    }

    void add(Statement *stmt) { statements.push_back(stmt); }

    virtual ~StatementList() {
	for (auto stmt : statements) {
	    if (stmt) delete stmt;
	}
    }

    void print() {
	for (auto stmt : statements) {
	    if (stmt) {
		printf("\n");
		stmt->print();
	    }
	}
	printf("\n");
    }

    std::vector<Statement *> statements;
};
