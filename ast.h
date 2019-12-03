#pragma once
#include <cstdio>
#include <map>
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
    enum Type { Print, Set, Expr };
    Statement()
        : type(Expr)
    {
    }

    Statement(Expression *other)
        : type(Print)
        , other(other)
    {
    }

    Statement(const std::string &pred, Expression *other)
        : type(Set)
        , pred(pred)
        , other(other)
    {
    }

    virtual ~Statement();

    virtual void print();
    virtual void exec(EvaluationContext &ec);

protected:
    Type        type;
    std::string pred;
    Expression *other = nullptr;
};

class Expression : public Statement {
public:
    virtual ~Expression() = default;
    void         exec(EvaluationContext &ec) {}
    virtual int  eval(EvaluationContext &ec) = 0;
    virtual void print(){};
};

class BinaryExpression : public Expression {
public:
    enum Type { And, Or, Impl, BiImpl };

    BinaryExpression(Expression *lhs, Expression *rhs, Type op)
        : lhs(lhs)
        , rhs(rhs)
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

    void print() override
    {
        printf("(");
        lhs->print();
        switch (op) {
        case Type::And:
            printf(" /\\ ");
            break;
        case Type::Or:
            printf(" \\/ ");
            break;
        case Type::Impl:
            printf(" -> ");
            break;
        case Type::BiImpl:
            printf(" <-> ");
            break;
        };
        rhs->print();
        printf(")");
    }

private:
    Type        op;
    Expression *lhs, *rhs;
};

class PredExpression : public Expression {
public:
    PredExpression(const std::string &name)
        : name(name)
    {
    }
    void print() override { printf("%s", name.c_str()); }
    int  eval(EvaluationContext &ec) override { return ec.predicates[name]; }

private:
    std::string name;
};

class ConstantExpression : public Expression {
public:
    ConstantExpression(bool value)
        : value(value)
    {
    }

    void print() override { printf("%s", value ? "tt" : "ff"); }
    int  eval(EvaluationContext &) override { return value; }

private:
    bool value;
};

class NegExpression : public Expression {
public:
    NegExpression(Expression *other)
        : other(other)
    {
    }

    ~NegExpression()
    {
        if (other)
            delete other;
    }
    void print() override
    {
        printf("(¬");
        other->print();
        printf(")");
    }

    int eval(EvaluationContext &ec) override { return !other->eval(ec); }

private:
    Expression *other = nullptr;
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
