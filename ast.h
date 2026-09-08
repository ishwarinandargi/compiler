#pragma once

#include "token.h"
#include <memory>
#include <vector>
#include <string>

// Forward declarations
class ASTVisitor;

/**
 * Base class for all AST nodes
 */
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

// Forward declarations for all node types to be used in Visitor
class NumberExpr;
class StringExpr;
class IdentifierExpr;
class BinaryExpr;
class UnaryExpr;
class CallExpr;

class BlockStmt;
class ExprStmt;
class VarDeclStmt;
class AssignStmt;
class BranchStmt;
class LoopStmt;
class ReturnStmt;

class FunctionDecl;
class EventDecl;
class StateDecl;
class ActionDecl;
class PropDecl;
class InitDecl;

class ProgramAST;

/**
 * Base Visitor interface for AST traversal
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expressions
    virtual void visitNumberExpr(NumberExpr& node) = 0;
    virtual void visitStringExpr(StringExpr& node) = 0;
    virtual void visitIdentifierExpr(IdentifierExpr& node) = 0;
    virtual void visitBinaryExpr(BinaryExpr& node) = 0;
    virtual void visitUnaryExpr(UnaryExpr& node) = 0;
    virtual void visitCallExpr(CallExpr& node) = 0;

    // Statements
    virtual void visitBlockStmt(BlockStmt& node) = 0;
    virtual void visitExprStmt(ExprStmt& node) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt& node) = 0;
    virtual void visitAssignStmt(AssignStmt& node) = 0;
    virtual void visitBranchStmt(BranchStmt& node) = 0;
    virtual void visitLoopStmt(LoopStmt& node) = 0;
    virtual void visitReturnStmt(ReturnStmt& node) = 0;

    // Declarations
    virtual void visitFunctionDecl(FunctionDecl& node) = 0;
    virtual void visitEventDecl(EventDecl& node) = 0;
    virtual void visitStateDecl(StateDecl& node) = 0;
    virtual void visitActionDecl(ActionDecl& node) = 0;
    virtual void visitPropDecl(PropDecl& node) = 0;
    virtual void visitInitDecl(InitDecl& node) = 0;

    // Program
    virtual void visitProgramAST(ProgramAST& node) = 0;
};

// --- Expressions ---

class Expr : public ASTNode {
};

class NumberExpr : public Expr {
public:
    Token token;
    double value;

    NumberExpr(Token tok, double val) : token(tok), value(val) {}
    void accept(ASTVisitor& visitor) override { visitor.visitNumberExpr(*this); }
};

class StringExpr : public Expr {
public:
    Token token;
    std::string value;

    StringExpr(Token tok, const std::string& val) : token(tok), value(val) {}
    void accept(ASTVisitor& visitor) override { visitor.visitStringExpr(*this); }
};

class IdentifierExpr : public Expr {
public:
    Token name;

    explicit IdentifierExpr(Token n) : name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visitIdentifierExpr(*this); }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    BinaryExpr(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBinaryExpr(*this); }
};

class UnaryExpr : public Expr {
public:
    Token op;
    std::unique_ptr<Expr> right;

    UnaryExpr(Token o, std::unique_ptr<Expr> r) : op(o), right(std::move(r)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitUnaryExpr(*this); }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    Token paren; // For line number info
    std::vector<std::unique_ptr<Expr>> arguments;

    CallExpr(std::unique_ptr<Expr> c, Token p, std::vector<std::unique_ptr<Expr>> args)
        : callee(std::move(c)), paren(p), arguments(std::move(args)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitCallExpr(*this); }
};

// --- Statements ---

class Stmt : public ASTNode {
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;

    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts) : statements(std::move(stmts)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBlockStmt(*this); }
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expression;

    explicit ExprStmt(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitExprStmt(*this); }
};

class VarDeclStmt : public Stmt {
public:
    Token typeKeyword; // CHAR, STATE, PROP
    Token name;
    std::unique_ptr<Expr> initializer; // Can be null

    VarDeclStmt(Token tk, Token n, std::unique_ptr<Expr> init)
        : typeKeyword(tk), name(n), initializer(std::move(init)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitVarDeclStmt(*this); }
};

class AssignStmt : public Stmt {
public:
    Token name;
    std::unique_ptr<Expr> value;

    AssignStmt(Token n, std::unique_ptr<Expr> val) : name(n), value(std::move(val)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitAssignStmt(*this); }
};

class BranchStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // Can be null

    BranchStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenB, std::unique_ptr<Stmt> elseB)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitBranchStmt(*this); }
};

class LoopStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    LoopStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b) : condition(std::move(cond)), body(std::move(b)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitLoopStmt(*this); }
};

class ReturnStmt : public Stmt {
public:
    Token keyword;
    std::unique_ptr<Expr> value; // Can be null

    ReturnStmt(Token kw, std::unique_ptr<Expr> val) : keyword(kw), value(std::move(val)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitReturnStmt(*this); }
};

// --- Declarations ---

class Decl : public Stmt {
};

class FunctionDecl : public Decl {
public:
    Token name;
    std::vector<Token> parameters;
    std::unique_ptr<BlockStmt> body;

    FunctionDecl(Token n, std::vector<Token> params, std::unique_ptr<BlockStmt> b)
        : name(n), parameters(std::move(params)), body(std::move(b)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitFunctionDecl(*this); }
};

class EventDecl : public Decl {
public:
    Token name;
    std::vector<Token> parameters; // Assuming events can have parameters
    std::unique_ptr<BlockStmt> body;

    EventDecl(Token n, std::vector<Token> params, std::unique_ptr<BlockStmt> b)
        : name(n), parameters(std::move(params)), body(std::move(b)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitEventDecl(*this); }
};

class StateDecl : public Decl {
public:
    Token name;
    std::unique_ptr<Expr> initializer; // Can be null

    StateDecl(Token n, std::unique_ptr<Expr> init) : name(n), initializer(std::move(init)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitStateDecl(*this); }
};

class ActionDecl : public Decl {
public:
    Token name;
    std::unique_ptr<BlockStmt> body;

    ActionDecl(Token n, std::unique_ptr<BlockStmt> b) : name(n), body(std::move(b)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitActionDecl(*this); }
};

class PropDecl : public Decl {
public:
    Token name;
    std::unique_ptr<Expr> initializer; // Can be null

    PropDecl(Token n, std::unique_ptr<Expr> init) : name(n), initializer(std::move(init)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitPropDecl(*this); }
};

class InitDecl : public Decl {
public:
    std::unique_ptr<BlockStmt> body;

    explicit InitDecl(std::unique_ptr<BlockStmt> b) : body(std::move(b)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitInitDecl(*this); }
};

// --- Program ---

class ProgramAST : public ASTNode {
public:
    std::vector<std::unique_ptr<Stmt>> declarations;

    explicit ProgramAST(std::vector<std::unique_ptr<Stmt>> decls) : declarations(std::move(decls)) {}
    void accept(ASTVisitor& visitor) override { visitor.visitProgramAST(*this); }
};

/**
 * A simple AST Printer that implements ASTVisitor to print the tree.
 */
class ASTPrinter : public ASTVisitor {
public:
    std::string print(ProgramAST& program);

    // Expressions
    void visitNumberExpr(NumberExpr& node) override;
    void visitStringExpr(StringExpr& node) override;
    void visitIdentifierExpr(IdentifierExpr& node) override;
    void visitBinaryExpr(BinaryExpr& node) override;
    void visitUnaryExpr(UnaryExpr& node) override;
    void visitCallExpr(CallExpr& node) override;

    // Statements
    void visitBlockStmt(BlockStmt& node) override;
    void visitExprStmt(ExprStmt& node) override;
    void visitVarDeclStmt(VarDeclStmt& node) override;
    void visitAssignStmt(AssignStmt& node) override;
    void visitBranchStmt(BranchStmt& node) override;
    void visitLoopStmt(LoopStmt& node) override;
    void visitReturnStmt(ReturnStmt& node) override;

    // Declarations
    void visitFunctionDecl(FunctionDecl& node) override;
    void visitEventDecl(EventDecl& node) override;
    void visitStateDecl(StateDecl& node) override;
    void visitActionDecl(ActionDecl& node) override;
    void visitPropDecl(PropDecl& node) override;
    void visitInitDecl(InitDecl& node) override;

    // Program
    void visitProgramAST(ProgramAST& node) override;

private:
    std::string result;
    std::vector<bool> isLastChildStack;
    
    void printIndent();
    void appendLine(const std::string& text);
};
