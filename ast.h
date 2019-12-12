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
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void operator()(Expression *){};
    virtual void operator()(class BinaryExpression *){};
    virtual void operator()(class NegExpression *){};
    virtual void operator()(class ConstantExpression *){};
    virtual void operator()(class PredExpression *){};
};

using ExpressionUPtr = std::unique_ptr<Expression>;

class Statement {
public:
    enum Type { Print, Set, Expr, PrintAtoms, PrintTable };
    Statement()
        : type(Expr)
    {
    }

    Statement(ExpressionUPtr other, Type type = Type::Print)
        : type(type)
        , other(std::move(other))
    {
    }

    Statement(const std::string &pred, ExpressionUPtr other)
        : type(Set)
        , pred(pred)
        , other(std::move(other))
    {
    }

    virtual ~Statement();

    virtual int  print();
    virtual void exec(EvaluationContext &ec);

protected:
    Type           type;
    std::string    pred;
    ExpressionUPtr other;
};

class Expression : public Statement {
public:
    Expression(Expression *parent)
        : parent(parent)
    {
    }
    virtual ~Expression() = default;
    void        exec(EvaluationContext &ec) {}
    virtual int eval(EvaluationContext &ec) = 0;
    virtual int print() { return 0; };

    virtual std::list<std::string>  atoms() const { return {}; };
    virtual std::list<Expression *> childs() const { return {}; };

    virtual void visit(Visitor &visitor) {}

    Expression *parent;
};

template<typename CRTP>
class CRTPExpression : public Expression {
public:
    template<typename... Args>
    CRTPExpression(Args &&... args)
        : Expression(std::forward<Args>(args)...)
    {
    }

    void visit(Visitor &visitor) override { visitor(static_cast<CRTP *>(this)); }
};

class BinaryExpression : public CRTPExpression<BinaryExpression> {
public:
    enum Type { And, Or, Impl, BiImpl };

    BinaryExpression(ExpressionUPtr lhs, ExpressionUPtr rhs, Type op, Expression *parent = nullptr)
        : CRTPExpression(parent)
        , lhs(std::move(lhs))
        , rhs(std::move(rhs))
        , op(op)
    {
        this->lhs->parent = this;
        this->rhs->parent = this;
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

    virtual std::list<Expression *> childs() const override { return { lhs.get(), rhs.get() }; }

private:
    Type           op;
    ExpressionUPtr lhs, rhs;
};

class PredExpression : public CRTPExpression<PredExpression> {
public:
    PredExpression(const std::string &name, Expression *parent = nullptr)
        : CRTPExpression(parent)
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

private:
    std::string name;
};

class ConstantExpression : public CRTPExpression<ConstantExpression> {
public:
    ConstantExpression(bool value, Expression *parent = nullptr)
        : CRTPExpression(parent)
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

private:
    bool value;
};

class NegExpression : public CRTPExpression<NegExpression> {
public:
    NegExpression(ExpressionUPtr other, Expression *parent = nullptr)
        : CRTPExpression(parent)
        , other(std::move(other))
    {
        this->other->parent = this;
    }

    int print() override
    {
        printf("(¬");
        const int c = other->print();
        printf(")");

        return 3 + c;
    }

    int eval(EvaluationContext &ec) override { return !other->eval(ec); }

    std::list<std::string> atoms() const override { return { other->atoms() }; }

    std::list<Expression *> childs() const override { return { other.get() }; }

private:
    ExpressionUPtr other;
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

class SimplePrintingASTWalker : public Visitor {
public:
    void operator()(BinaryExpression *be) override
    {
        printf("Found binary expression!\n");
        for (auto &&c : be->childs()) {
            c->visit(*this);
        }
    }

    void operator()(NegExpression *ne) override
    {
        printf("Found negexpr\n");
        for (auto &&c : ne->childs()) {
            c->visit(*this);
        }
    }

    void operator()(ConstantExpression *ce) override
    {
        printf("Found const expr\n");
        for (auto &&c : ce->childs()) {
            c->visit(*this);
        }
    }

    void operator()(PredExpression *pe) override
    {
        printf("Found predexpr\n");
        for (auto &&c : pe->childs()) {
            c->visit(*this);
        }
    }
};
