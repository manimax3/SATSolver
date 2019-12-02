#pragma once
#include <cstdio>
#include <map>
#include <string>
#include <vector>

struct EvaluationContext {
    std::map<std::string, bool> predicates;
};

class Statement {
   public:
    virtual ~Statement() = default;
    virtual void print(){};
    virtual void exec(EvaluationContext &ec) {}
};

class SetStatement : public Statement {
   public:
    SetStatement(const std::string &pred, bool value)
	: pred(pred), value(value) {}

    void print() override { printf("Setting %s to %i", pred.c_str(), value); }
    virtual void exec(EvaluationContext &ec) { ec.predicates[pred] = value; }

   private:
    std::string pred;
    bool value;
};

class Expression : public Statement {
   public:
    virtual ~Expression() = default;
    void exec(EvaluationContext &ec) {}
    virtual int eval(EvaluationContext &ec) = 0;
    virtual void print(){};
};

class PredExpression : public Expression {
   public:
    PredExpression(const std::string &name) : name(name) {}
    void print() override { printf("%s", name.c_str()); }
    int eval(EvaluationContext &ec) override { return ec.predicates[name]; }

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

    int eval(EvaluationContext &ec) override {
	return (!lhs->eval(ec)) || rhs->eval(ec);
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

    int eval(EvaluationContext &ec) override {
	const auto a = lhs->eval(ec);
	const auto b = rhs->eval(ec);

	return a == b;
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

    int eval(EvaluationContext &ec) override {
	return lhs->eval(ec) && rhs->eval(ec);
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

    int eval(EvaluationContext &ec) override {
	return lhs->eval(ec) || rhs->eval(ec);
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

    int eval(EvaluationContext &ec) override { return !other->eval(ec); }

   private:
    Expression *other = nullptr;
};

class PrintStatement : public Statement {
   public:
    PrintStatement(Expression *expr) : expr(expr) {}

    void print() override {
	printf("Printing the result of: ");
	expr->print();
    }

    void exec(EvaluationContext &ec) override {
	expr->print();
	printf(" ==> %i\n", expr->eval(ec));
    }

    ~PrintStatement() {
	if (expr) delete expr;
    }

   private:
    Expression *expr = nullptr;
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

    void run() {
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
