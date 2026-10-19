#pragma once

#include "ast.h"
#include "ir.h"
#include <string>

/**
 * Three-Address Code Generator for StoryLang
 * Implements the AST Visitor pattern to traverse the syntax tree
 * and generate clean, college-level 3AC quadruples.
 */
class CodeGenerator : public Visitor
{
public:
    CodeGenerator() = default;

    // Entry point: translates an entire AST Program into 3AC
    void generate(const Program &program);

    // Access the generated 3AC
    const ThreeAddressCode &code() const { return code_; }
    ThreeAddressCode &code() { return code_; }

    // Reset counters and state
    void reset();

    // AST Visitor methods
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
    ThreeAddressCode code_;
    int tempCount_ = 0;
    int labelCount_ = 0;
    std::string lastTemp_;

    // Generate fresh temporary name: t0, t1, t2, ...
    std::string newTemp();

    // Generate fresh label name: L1, L2, L3, ...
    std::string newLabel();
};
