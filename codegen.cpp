#include "codegen.h"

void CodeGenerator::reset()
{
    code_.clear();
    tempCount_ = 0;
    labelCount_ = 0;
    lastTemp_.clear();
}

std::string CodeGenerator::newTemp()
{
    return "t" + std::to_string(tempCount_++);
}

std::string CodeGenerator::newLabel()
{
    return "L" + std::to_string(++labelCount_);
}

void CodeGenerator::generate(const Program &program)
{
    reset();

    for (const auto &decl : program.declarations)
    {
        decl->accept(*this);
    }

    if (!program.statements.empty())
    {
        code_.emit("FUNC_BEGIN", "main", "", "");
        for (const auto &stmt : program.statements)
        {
            stmt->accept(*this);
        }
        code_.emit("FUNC_END", "main", "", "");
    }
}

void CodeGenerator::visit(IdentifierExpr &expr)
{
    lastTemp_ = expr.name;
}

void CodeGenerator::visit(LiteralExpr &expr)
{
    if (expr.type == TokenType::STRING)
    {
        lastTemp_ = "\"" + expr.value + "\"";
    }
    else
    {
        lastTemp_ = expr.value;
    }
}

void CodeGenerator::visit(BinaryExpr &expr)
{
    expr.left->accept(*this);
    std::string left = lastTemp_;

    expr.right->accept(*this);
    std::string right = lastTemp_;

    std::string temp = newTemp();
    code_.emitBinary(expr.op, temp, left, right);
    lastTemp_ = temp;
}

void CodeGenerator::visit(UnaryExpr &expr)
{
    expr.operand->accept(*this);
    std::string operand = lastTemp_;

    std::string temp = newTemp();
    if (expr.op == "-")
    {
        code_.emitUnary("NEG", temp, operand);
    }
    else if (expr.op == "!")
    {
        code_.emitUnary("NOT", temp, operand);
    }
    else
    {
        code_.emitUnary(expr.op, temp, operand);
    }
    lastTemp_ = temp;
}

void CodeGenerator::visit(CallExpr &expr)
{
    std::vector<std::string> args;
    for (const auto &arg : expr.args)
    {
        arg->accept(*this);
        args.push_back(lastTemp_);
    }

    for (const auto &arg : args)
    {
        code_.emitParam(arg);
    }

    std::string temp = newTemp();
    code_.emitCall(expr.callee, static_cast<int>(args.size()), temp);
    lastTemp_ = temp;
}

void CodeGenerator::visit(ActionStmt &stmt)
{
    std::vector<std::string> args;
    for (const auto &arg : stmt.args)
    {
        arg->accept(*this);
        args.push_back(lastTemp_);
    }

    for (const auto &arg : args)
    {
        code_.emitParam(arg);
    }

    code_.emitAction(stmt.name, static_cast<int>(args.size()));
}

void CodeGenerator::visit(BranchStmt &stmt)
{
    stmt.condition->accept(*this);
    std::string cond = lastTemp_;

    std::string labelElse = newLabel();
    std::string labelEnd = newLabel();

    if (!stmt.elseBlock.empty())
    {
        code_.emitIfFalse(cond, labelElse);
        for (const auto &s : stmt.thenBlock)
        {
            s->accept(*this);
        }
        code_.emitGoto(labelEnd);
        code_.emitLabel(labelElse);
        for (const auto &s : stmt.elseBlock)
        {
            s->accept(*this);
        }
        code_.emitLabel(labelEnd);
    }
    else
    {
        code_.emitIfFalse(cond, labelEnd);
        for (const auto &s : stmt.thenBlock)
        {
            s->accept(*this);
        }
        code_.emitLabel(labelEnd);
    }
}

void CodeGenerator::visit(LoopStmt &stmt)
{
    std::string labelStart = newLabel();
    std::string labelEnd = newLabel();

    code_.emitLabel(labelStart);
    stmt.condition->accept(*this);
    std::string cond = lastTemp_;

    code_.emitIfFalse(cond, labelEnd);
    for (const auto &s : stmt.body)
    {
        s->accept(*this);
    }
    code_.emitGoto(labelStart);
    code_.emitLabel(labelEnd);
}

void CodeGenerator::visit(AssignmentStmt &stmt)
{
    stmt.value->accept(*this);
    code_.emitAssignment(stmt.lvalue, lastTemp_);
}

void CodeGenerator::visit(ReturnStmt &stmt)
{
    if (stmt.value)
    {
        stmt.value->accept(*this);
        code_.emitReturn(lastTemp_);
    }
    else
    {
        code_.emitReturn();
    }
}

void CodeGenerator::visit(CharDecl &decl)
{
    code_.emit("CHAR_DECL", decl.name, decl.type, "");
}

void CodeGenerator::visit(StateDecl &decl)
{
    std::string vals;
    for (size_t i = 0; i < decl.values.size(); ++i)
    {
        vals += decl.values[i];
        if (i + 1 < decl.values.size()) vals += " | ";
    }
    code_.emit("STATE_DECL", decl.name, vals, "");
}

void CodeGenerator::visit(EventDecl &decl)
{
    code_.emit("EVENT_BEGIN", decl.name, "", "");
    for (const auto &s : decl.body)
    {
        s->accept(*this);
    }
    code_.emit("EVENT_END", decl.name, "", "");
}

void CodeGenerator::visit(FuncDecl &decl)
{
    code_.emit("FUNC_BEGIN", decl.name, "", "");
    for (const auto &param : decl.params)
    {
        code_.emit("PARAM", param, "", "");
    }
    for (const auto &s : decl.body)
    {
        s->accept(*this);
    }
    code_.emit("FUNC_END", decl.name, "", "");
}
