#include "ast.h"

std::string ASTPrinter::print(const Program &program)
{
    result_.clear();
    indent_ = 0;

    line("Program");
    indent_++;

    for (const auto &decl : program.declarations)
    {
        decl->accept(*this);
    }

    for (const auto &stmt : program.statements)
    {
        stmt->accept(*this);
    }

    indent_--;
    return result_;
}

void ASTPrinter::indent()
{
    for (int i = 0; i < indent_; ++i)
    {
        result_ += "  ";
    }
}

void ASTPrinter::line(const std::string &text)
{
    indent();
    result_ += text + "\n";
}

void ASTPrinter::visit(IdentifierExpr &expr)
{
    line("Identifier: " + expr.name);
}

void ASTPrinter::visit(LiteralExpr &expr)
{
    line("Literal: " + expr.value);
}

void ASTPrinter::visit(BinaryExpr &expr)
{
    line("BinaryExpr: " + expr.op);
    indent_++;
    expr.left->accept(*this);
    expr.right->accept(*this);
    indent_--;
}

void ASTPrinter::visit(UnaryExpr &expr)
{
    line("UnaryExpr: " + expr.op);
    indent_++;
    expr.operand->accept(*this);
    indent_--;
}

void ASTPrinter::visit(CallExpr &expr)
{
    line("CallExpr: " + expr.callee);
    indent_++;
    for (const auto &arg : expr.args)
    {
        arg->accept(*this);
    }
    indent_--;
}

void ASTPrinter::visit(ActionStmt &stmt)
{
    line("ActionStmt: " + stmt.name);
    indent_++;
    for (const auto &arg : stmt.args)
    {
        arg->accept(*this);
    }
    indent_--;
}

void ASTPrinter::visit(BranchStmt &stmt)
{
    line("BranchStmt");
    indent_++;
    line("Condition:");
    indent_++;
    stmt.condition->accept(*this);
    indent_--;

    line("Then:");
    indent_++;
    for (const auto &s : stmt.thenBlock)
    {
        s->accept(*this);
    }
    indent_--;

    if (!stmt.elseBlock.empty())
    {
        line("Else:");
        indent_++;
        for (const auto &s : stmt.elseBlock)
        {
            s->accept(*this);
        }
        indent_--;
    }
    indent_--;
}

void ASTPrinter::visit(LoopStmt &stmt)
{
    line("LoopStmt");
    indent_++;
    line("Condition:");
    indent_++;
    stmt.condition->accept(*this);
    indent_--;

    line("Body:");
    indent_++;
    for (const auto &s : stmt.body)
    {
        s->accept(*this);
    }
    indent_--;
    indent_--;
}

void ASTPrinter::visit(AssignmentStmt &stmt)
{
    line("AssignmentStmt: " + stmt.lvalue + " :=");
    indent_++;
    stmt.value->accept(*this);
    indent_--;
}

void ASTPrinter::visit(ReturnStmt &stmt)
{
    line("ReturnStmt");
    if (stmt.value)
    {
        indent_++;
        stmt.value->accept(*this);
        indent_--;
    }
}

void ASTPrinter::visit(CharDecl &decl)
{
    line("CharDecl: " + decl.name + " (" + decl.type + ")");
}

void ASTPrinter::visit(StateDecl &decl)
{
    std::string vals;
    for (size_t i = 0; i < decl.values.size(); ++i)
    {
        vals += decl.values[i];
        if (i + 1 < decl.values.size()) vals += ", ";
    }
    line("StateDecl: " + decl.name + " [" + vals + "]");
}

void ASTPrinter::visit(EventDecl &decl)
{
    line("EventDecl: " + decl.name);
    indent_++;
    for (const auto &s : decl.body)
    {
        s->accept(*this);
    }
    indent_--;
}

void ASTPrinter::visit(FuncDecl &decl)
{
    std::string params;
    for (size_t i = 0; i < decl.params.size(); ++i)
    {
        params += decl.params[i];
        if (i + 1 < decl.params.size()) params += ", ";
    }
    line("FuncDecl: " + decl.name + "(" + params + ")");
    indent_++;
    for (const auto &s : decl.body)
    {
        s->accept(*this);
    }
    indent_--;
}
