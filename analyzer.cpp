#include "analyzer.h"

#include <sstream>
#include <utility>

SemanticAnalyzer::SemanticAnalyzer() = default;

void SemanticAnalyzer::analyze(const Program &program)
{
    symbolTable_.enterScope();

    for (const auto &decl : program.declarations)
    {
        decl->accept(*this);
    }

    for (const auto &stmt : program.statements)
    {
        stmt->accept(*this);
    }
}

void SemanticAnalyzer::error(const std::string &message, int line, int column)
{
    std::ostringstream oss;
    oss << "error: " << message << " at line " << line << ", column " << column;
    errors_.push_back(oss.str());
}

void SemanticAnalyzer::note(const std::string &message, int line, int column)
{
    std::ostringstream oss;
    oss << "note: " << message << " at line " << line << ", column " << column;
    errors_.push_back(oss.str());
}

std::string SemanticAnalyzer::inferType(const Expr &expr)
{
    switch (expr.kind)
    {
    case NodeKind::Identifier:
    {
        const auto &ident = static_cast<const IdentifierExpr &>(expr);
        return inferType(ident.name);
    }
    case NodeKind::Literal:
    {
        const auto &literal = static_cast<const LiteralExpr &>(expr);
        if (literal.type == TokenType::NUMBER)
        {
            return "number";
        }
        if (literal.type == TokenType::STRING)
        {
            return "string";
        }
        return "unknown";
    }
    case NodeKind::Binary:
    {
        const auto &binary = static_cast<const BinaryExpr &>(expr);
        const std::string left = inferType(*binary.left);
        const std::string right = inferType(*binary.right);

        if (binary.op == "==" || binary.op == "!=" || binary.op == "<" || binary.op == "<=" ||
            binary.op == ">" || binary.op == ">=" || binary.op == "&&" || binary.op == "||")
        {
            return "boolean";
        }
        if (left == right && (left == "number" || left == "string"))
        {
            return left;
        }
        return "unknown";
    }
    case NodeKind::Unary:
    {
        const auto &unary = static_cast<const UnaryExpr &>(expr);
        if (unary.op == "!" || unary.op == "-")
        {
            return inferType(*unary.operand);
        }
        return "unknown";
    }
    case NodeKind::Call:
    {
        const auto &call = static_cast<const CallExpr &>(expr);
        if (call.callee == "AI.choose")
        {
            return "number";
        }
        const auto *sym = symbolTable_.lookup(call.callee);
        if (!sym)
        {
            return "unknown";
        }
        return sym->dataType;
    }
    default:
        return "unknown";
    }
}

std::string SemanticAnalyzer::inferType(const std::string &name)
{
    const Symbol *sym = symbolTable_.lookup(name);
    if (!sym)
    {
        return "unknown";
    }
    return sym->dataType;
}

bool SemanticAnalyzer::isCompatible(const std::string &left, const std::string &right) const
{
    return left == right || left == "unknown" || right == "unknown";
}

bool SemanticAnalyzer::isBoolean(const std::string &type) const
{
    return type == "boolean";
}

bool SemanticAnalyzer::isIntegerLike(const std::string &type) const
{
    return type == "number" || type == "int" || type == "float";
}

bool SemanticAnalyzer::isStringLike(const std::string &type) const
{
    return type == "string";
}

bool SemanticAnalyzer::isBuiltinFunction(const std::string &name) const
{
    return name == "AI.choose";
}

bool SemanticAnalyzer::isBuiltinAction(const std::string &name) const
{
    return name == "AI.generate";
}

void SemanticAnalyzer::validateAssignment(const std::string &lvalue, const std::string &rvalue, int line, int column)
{
    const std::string leftType = inferType(lvalue);
    if (leftType == "unknown")
    {
        error("Undefined identifier '" + lvalue + "'", line, column);
        return;
    }

    if (!isCompatible(leftType, rvalue))
    {
        error("Type mismatch: cannot assign " + rvalue + " to " + leftType, line, column);
    }
}

void SemanticAnalyzer::validateFunctionCall(const CallExpr &call, int line, int column)
{
    if (isBuiltinFunction(call.callee))
    {
        if (call.args.size() < 2)
        {
            error("AI.choose requires a prompt and at least one option", line, column);
        }
        return;
    }

    const Symbol *fn = symbolTable_.lookup(call.callee);
    if (!fn)
    {
        error("Undefined function '" + call.callee + "'", line, column);
        return;
    }

    if (fn->parameters.size() != call.args.size())
    {
        error("Wrong argument count for '" + call.callee + "'", line, column);
    }
}

std::vector<std::string> SemanticAnalyzer::extractStateValues(const StateDecl &decl) const
{
    return decl.values;
}

void SemanticAnalyzer::visit(IdentifierExpr &expr)
{
    if (symbolTable_.lookup(expr.name) == nullptr)
    {
        error("Undefined reference to '" + expr.name + "'", 0, 0);
    }
}

void SemanticAnalyzer::visit(LiteralExpr &expr)
{
    (void)expr;
}

void SemanticAnalyzer::visit(BinaryExpr &expr)
{
    const std::string leftType = inferType(*expr.left);
    const std::string rightType = inferType(*expr.right);

    if (expr.op == "&&" || expr.op == "||")
    {
        if (!isBoolean(leftType) || !isBoolean(rightType))
        {
            error("Operator '" + expr.op + "' requires boolean operands", 0, 0);
        }
    }
    else if (expr.op == "==" || expr.op == "!=" || expr.op == "<" || expr.op == "<=" ||
             expr.op == ">" || expr.op == ">=")
    {
        if (!isCompatible(leftType, rightType))
        {
            error("Type mismatch in comparison", 0, 0);
        }
    }
}

void SemanticAnalyzer::visit(UnaryExpr &expr)
{
    const std::string operandType = inferType(*expr.operand);
    if (expr.op == "!" && !isBoolean(operandType))
    {
        error("Unary '!' requires boolean operand", 0, 0);
    }
    else if (expr.op == "-" && !isIntegerLike(operandType))
    {
        error("Unary '-' requires numeric operand", 0, 0);
    }
}

void SemanticAnalyzer::visit(CallExpr &expr)
{
    validateFunctionCall(expr, 0, 0);
    for (const auto &arg : expr.args)
    {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ActionStmt &stmt)
{
    if (isBuiltinAction(stmt.name))
    {
        if (stmt.args.size() != 1)
        {
            error("AI.generate requires exactly one prompt", 0, 0);
        }
        for (const auto &arg : stmt.args)
        {
            arg->accept(*this);
        }
        return;
    }

    for (const auto &arg : stmt.args)
    {
        if (const auto *ident = dynamic_cast<const IdentifierExpr *>(arg.get()))
        {
            if (symbolTable_.lookup(ident->name) == nullptr)
            {
                error("Undefined reference to '" + ident->name + "'", 0, 0);
            }
        }
        else
        {
            arg->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(BranchStmt &stmt)
{
    if (stmt.condition)
    {
        const std::string type = inferType(*stmt.condition);
        if (!isBoolean(type))
        {
            error("Branch condition must be boolean", 0, 0);
        }
    }
    for (const auto &child : stmt.thenBlock)
    {
        child->accept(*this);
    }
    for (const auto &child : stmt.elseBlock)
    {
        child->accept(*this);
    }
}

void SemanticAnalyzer::visit(LoopStmt &stmt)
{
    if (stmt.condition)
    {
        const std::string type = inferType(*stmt.condition);
        if (!isBoolean(type))
        {
            error("Loop condition must be boolean", 0, 0);
        }
    }
    for (const auto &child : stmt.body)
    {
        child->accept(*this);
    }
}

void SemanticAnalyzer::visit(AssignmentStmt &stmt)
{
    const std::string valueType = inferType(*stmt.value);
    const std::string leftType = inferType(stmt.lvalue);
    const Symbol *lhs = symbolTable_.lookup(stmt.lvalue);

    if (leftType == "unknown")
    {
        error("Undefined identifier '" + stmt.lvalue + "'", 0, 0);
        return;
    }

    if (lhs && lhs->kind == Symbol::Kind::State)
    {
        const auto valuesIt = lhs->attributes.find("values");
        if (valuesIt != lhs->attributes.end())
        {
            std::string values = valuesIt->second;
            std::string val;
            if (auto *ident = dynamic_cast<IdentifierExpr *>(stmt.value.get()))
            {
                val = ident->name;
            }
            else if (auto *literal = dynamic_cast<LiteralExpr *>(stmt.value.get()))
            {
                val = literal->value;
            }

            if (!val.empty() && values.find(val) == std::string::npos)
            {
                error("Invalid enum value '" + val + "' for state '" + stmt.lvalue + "'", 0, 0);
            }
        }
    }
    else if (!isCompatible(leftType, valueType))
    {
        error("Type mismatch: cannot assign " + valueType + " to " + leftType, 0, 0);
    }
}

void SemanticAnalyzer::visit(ReturnStmt &stmt)
{
    if (stmt.value)
    {
        const std::string valueType = inferType(*stmt.value);
        if (!currentFunction_.empty())
        {
            const Symbol *fn = symbolTable_.lookup(currentFunction_);
            if (fn && !isCompatible(fn->dataType, valueType))
            {
                error("Return type mismatch", 0, 0);
            }
        }
    }
}

void SemanticAnalyzer::visit(CharDecl &decl)
{
    if (symbolTable_.isDefined(decl.name))
    {
        error("Duplicate declaration of '" + decl.name + "'", 0, 0);
        return;
    }

    Symbol symbol;
    symbol.name = decl.name;
    symbol.kind = Symbol::Kind::Char;
    symbol.dataType = decl.type;
    symbol.line = 0;
    symbol.column = 0;
    symbolTable_.define(symbol);
}

void SemanticAnalyzer::visit(StateDecl &decl)
{
    if (symbolTable_.isDefined(decl.name))
    {
        error("Duplicate declaration of '" + decl.name + "'", 0, 0);
        return;
    }

    Symbol symbol;
    symbol.name = decl.name;
    symbol.kind = Symbol::Kind::State;
    symbol.dataType = "enum";
    symbol.attributes["values"] = "";
    for (const auto &value : decl.values)
    {
        symbol.attributes["values"] += value + ",";
    }
    symbolTable_.define(symbol);
}

void SemanticAnalyzer::visit(EventDecl &decl)
{
    if (symbolTable_.isDefined(decl.name))
    {
        error("Duplicate declaration of '" + decl.name + "'", 0, 0);
        return;
    }

    Symbol symbol;
    symbol.name = decl.name;
    symbol.kind = Symbol::Kind::Func;
    symbol.dataType = "event";
    symbolTable_.define(symbol);

    for (const auto &stmt : decl.body)
    {
        stmt->accept(*this);
    }
}

void SemanticAnalyzer::visit(FuncDecl &decl)
{
    if (symbolTable_.isDefined(decl.name))
    {
        error("Duplicate declaration of '" + decl.name + "'", 0, 0);
        return;
    }

    Symbol symbol;
    symbol.name = decl.name;
    symbol.kind = Symbol::Kind::Func;
    symbol.dataType = "function";
    symbol.parameters.reserve(decl.params.size());
    for (const auto &param : decl.params)
    {
        Symbol paramSym;
        paramSym.name = param;
        paramSym.kind = Symbol::Kind::Var;
        paramSym.dataType = "unknown";
        symbol.parameters.push_back(paramSym);
    }
    symbolTable_.define(symbol);

    currentFunction_ = decl.name;
    inFunction_ = true;
    symbolTable_.enterScope();

    for (const auto &param : decl.params)
    {
        Symbol p;
        p.name = param;
        p.kind = Symbol::Kind::Var;
        p.dataType = "unknown";
        p.initialized = true;
        symbolTable_.define(p);
    }

    for (const auto &stmt : decl.body)
    {
        stmt->accept(*this);
    }

    symbolTable_.exitScope();
    inFunction_ = false;
    currentFunction_ = "";
}

