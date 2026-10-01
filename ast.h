#pragma once

#include "token.h"
#include <memory>
#include <string>
#include <utility>
#include <vector>

class Visitor;

enum class NodeKind
{
    Program,
    Identifier,
    Literal,
    Binary,
    Unary,
    Call,
    Action,
    Branch,
    Loop,
    Assignment,
    Return,
    CharDecl,
    StateDecl,
    EventDecl,
    FuncDecl
};

struct Expr
{
    explicit Expr(NodeKind kind) : kind(kind) {}
    NodeKind kind;
    virtual ~Expr() = default;
    virtual void accept(Visitor &visitor) = 0;
};

struct Statement
{
    explicit Statement(NodeKind kind) : kind(kind) {}
    NodeKind kind;
    virtual ~Statement() = default;
    virtual void accept(Visitor &visitor) = 0;
};

struct Declaration
{
    explicit Declaration(NodeKind kind) : kind(kind) {}
    NodeKind kind;
    virtual ~Declaration() = default;
    virtual void accept(Visitor &visitor) = 0;
};

struct IdentifierExpr : Expr
{
    explicit IdentifierExpr(std::string name)
        : Expr{NodeKind::Identifier}, name(std::move(name)) {}

    std::string name;
    void accept(Visitor &visitor) override;
};

struct LiteralExpr : Expr
{
    LiteralExpr(TokenType type, std::string value)
        : Expr{NodeKind::Literal}, type(type), value(std::move(value)) {}

    TokenType type;
    std::string value;
    void accept(Visitor &visitor) override;
};

struct BinaryExpr : Expr
{
    BinaryExpr(std::unique_ptr<Expr> left, std::string op, std::unique_ptr<Expr> right)
        : Expr{NodeKind::Binary}, left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    std::string op;
    std::unique_ptr<Expr> right;
    void accept(Visitor &visitor) override;
};

struct UnaryExpr : Expr
{
    UnaryExpr(std::string op, std::unique_ptr<Expr> operand)
        : Expr{NodeKind::Unary}, op(std::move(op)), operand(std::move(operand)) {}

    std::string op;
    std::unique_ptr<Expr> operand;
    void accept(Visitor &visitor) override;
};

struct CallExpr : Expr
{
    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args)
        : Expr{NodeKind::Call}, callee(std::move(callee)), args(std::move(args)) {}

    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    void accept(Visitor &visitor) override;
};

struct ActionStmt : Statement
{
    ActionStmt(std::string name, std::vector<std::unique_ptr<Expr>> args)
        : Statement{NodeKind::Action}, name(std::move(name)), args(std::move(args)) {}

    std::string name;
    std::vector<std::unique_ptr<Expr>> args;
    void accept(Visitor &visitor) override;
};

struct BranchStmt : Statement
{
    BranchStmt(std::unique_ptr<Expr> condition,
               std::vector<std::unique_ptr<Statement>> thenBlock,
               std::vector<std::unique_ptr<Statement>> elseBlock)
        : Statement{NodeKind::Branch},
          condition(std::move(condition)),
          thenBlock(std::move(thenBlock)),
          elseBlock(std::move(elseBlock)) {}

    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Statement>> thenBlock;
    std::vector<std::unique_ptr<Statement>> elseBlock;
    void accept(Visitor &visitor) override;
};

struct LoopStmt : Statement
{
    LoopStmt(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<Statement>> body)
        : Statement{NodeKind::Loop}, condition(std::move(condition)), body(std::move(body)) {}

    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<Statement>> body;
    void accept(Visitor &visitor) override;
};

struct AssignmentStmt : Statement
{
    AssignmentStmt(std::string lvalue, std::unique_ptr<Expr> value)
        : Statement{NodeKind::Assignment}, lvalue(std::move(lvalue)), value(std::move(value)) {}

    std::string lvalue;
    std::unique_ptr<Expr> value;
    void accept(Visitor &visitor) override;
};

struct ReturnStmt : Statement
{
    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : Statement{NodeKind::Return}, value(std::move(value)) {}

    std::unique_ptr<Expr> value;
    void accept(Visitor &visitor) override;
};

struct CharDecl : Declaration
{
    CharDecl(std::string name, std::string type)
        : Declaration{NodeKind::CharDecl}, name(std::move(name)), type(std::move(type)) {}

    std::string name;
    std::string type;
    void accept(Visitor &visitor) override;
};

struct StateDecl : Declaration
{
    StateDecl(std::string name, std::vector<std::string> values)
        : Declaration{NodeKind::StateDecl}, name(std::move(name)), values(std::move(values)) {}

    std::string name;
    std::vector<std::string> values;
    void accept(Visitor &visitor) override;
};

struct EventDecl : Declaration
{
    EventDecl(std::string name, std::vector<std::unique_ptr<Statement>> body)
        : Declaration{NodeKind::EventDecl}, name(std::move(name)), body(std::move(body)) {}

    std::string name;
    std::vector<std::unique_ptr<Statement>> body;
    void accept(Visitor &visitor) override;
};

struct FuncDecl : Declaration
{
    FuncDecl(std::string name,
             std::vector<std::string> params,
             std::vector<std::unique_ptr<Statement>> body)
        : Declaration{NodeKind::FuncDecl}, name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

    std::string name;
    std::vector<std::string> params;
    std::vector<std::unique_ptr<Statement>> body;
    void accept(Visitor &visitor) override;
};

struct Program
{
    std::vector<std::unique_ptr<Declaration>> declarations;
    std::vector<std::unique_ptr<Statement>> statements;
};

struct Visitor
{
    virtual ~Visitor() = default;
    virtual void visit(IdentifierExpr &expr) = 0;
    virtual void visit(LiteralExpr &expr) = 0;
    virtual void visit(BinaryExpr &expr) = 0;
    virtual void visit(UnaryExpr &expr) = 0;
    virtual void visit(CallExpr &expr) = 0;
    virtual void visit(ActionStmt &stmt) = 0;
    virtual void visit(BranchStmt &stmt) = 0;
    virtual void visit(LoopStmt &stmt) = 0;
    virtual void visit(AssignmentStmt &stmt) = 0;
    virtual void visit(ReturnStmt &stmt) = 0;
    virtual void visit(CharDecl &decl) = 0;
    virtual void visit(StateDecl &decl) = 0;
    virtual void visit(EventDecl &decl) = 0;
    virtual void visit(FuncDecl &decl) = 0;
};
