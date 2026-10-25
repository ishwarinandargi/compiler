#pragma once

#include "ast.h"
#include "symbol_table.h"

#include <string>
#include <vector>

class SemanticAnalyzer : public Visitor
{
public:
    SemanticAnalyzer();

    void analyze(const Program &program);
    const std::vector<std::string> &errors() const { return errors_; }
    const SymbolTable &symbolTable() const { return symbolTable_; }

    void visit(IdentifierExpr &expr) override;
    void visit(LiteralExpr &expr) override;
    void visit(BinaryExpr &expr) override;
    void visit(UnaryExpr &expr) override;
    void visit(CallExpr &expr) override;
    void visit(ActionStmt &stmt) override;
    void visit(BranchStmt &stmt) override;
    void visit(LoopStmt &stmt) override;
    void visit(AssignmentStmt &stmt) override;
    void visit(ReturnStmt &stmt) override;
    void visit(CharDecl &decl) override;
    void visit(StateDecl &decl) override;
    void visit(EventDecl &decl) override;
    void visit(FuncDecl &decl) override;

private:
    SymbolTable symbolTable_;
    std::vector<std::string> errors_;
    std::string currentFunction_;
    std::string currentType_;
    bool inFunction_ = false;

    void error(const std::string &message, int line, int column);
    void note(const std::string &message, int line, int column);
    std::string inferType(const Expr &expr);
    std::string inferType(const std::string &name);
    bool isCompatible(const std::string &left, const std::string &right) const;
    bool isBoolean(const std::string &type) const;
    bool isIntegerLike(const std::string &type) const;
    bool isStringLike(const std::string &type) const;
    bool isBuiltinFunction(const std::string &name) const;
    bool isBuiltinAction(const std::string &name) const;
    void validateAssignment(const std::string &lvalue, const std::string &rvalue, int line, int column);
    void validateFunctionCall(const CallExpr &call, int line, int column);
    std::vector<std::string> extractStateValues(const StateDecl &decl) const;
};
