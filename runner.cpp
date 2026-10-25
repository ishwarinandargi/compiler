#include "runner.h"

#include "ai_runtime.h"

#include <iostream>
#include <stdexcept>
#include <utility>

StoryRuntime::Value StoryRuntime::Value::numberValue(int value)
{
    Value result;
    result.kind = Kind::Number;
    result.number = value;
    return result;
}

StoryRuntime::Value StoryRuntime::Value::stringValue(std::string value)
{
    Value result;
    result.kind = Kind::String;
    result.text = std::move(value);
    return result;
}

StoryRuntime::Value StoryRuntime::Value::booleanValue(bool value)
{
    Value result;
    result.kind = Kind::Boolean;
    result.boolean = value;
    return result;
}

void StoryRuntime::run(const Program &program)
{
    values_.clear();
    enumValues_.clear();

    for (const auto &declaration : program.declarations)
    {
        if (const auto *state = dynamic_cast<const StateDecl *>(declaration.get()))
        {
            for (std::size_t i = 0; i < state->values.size(); ++i)
            {
                enumValues_[state->values[i]] = static_cast<int>(i);
            }
        }
        else if (const auto *character = dynamic_cast<const CharDecl *>(declaration.get()))
        {
            values_[character->name] = Value::stringValue(character->type);
        }
    }

    executeBlock(program.statements);
}

StoryRuntime::Value StoryRuntime::evaluate(const Expr &expr) const
{
    switch (expr.kind)
    {
    case NodeKind::Identifier:
    {
        const auto &identifier = static_cast<const IdentifierExpr &>(expr);
        const auto value = values_.find(identifier.name);
        if (value != values_.end()) return value->second;

        const auto enumValue = enumValues_.find(identifier.name);
        if (enumValue != enumValues_.end()) return Value::numberValue(enumValue->second);
        return Value::stringValue(identifier.name);
    }
    case NodeKind::Literal:
    {
        const auto &literal = static_cast<const LiteralExpr &>(expr);
        if (literal.type == TokenType::NUMBER) return Value::numberValue(std::stoi(literal.value));
        return Value::stringValue(literal.value);
    }
    case NodeKind::Call:
        return evaluateCall(static_cast<const CallExpr &>(expr));
    case NodeKind::Binary:
        return evaluateBinary(static_cast<const BinaryExpr &>(expr));
    case NodeKind::Unary:
        return evaluateUnary(static_cast<const UnaryExpr &>(expr));
    default:
        throw std::runtime_error("Unsupported expression in runtime");
    }
}

StoryRuntime::Value StoryRuntime::evaluateCall(const CallExpr &call) const
{
    if (call.callee != "AI.choose")
    {
        throw std::runtime_error("Unsupported runtime function: " + call.callee);
    }

    std::vector<std::string> arguments;
    for (const auto &argument : call.args)
    {
        if (const auto *identifier = dynamic_cast<const IdentifierExpr *>(argument.get()))
        {
            arguments.push_back(identifier->name);
        }
        else
        {
            arguments.push_back(asText(evaluate(*argument)));
        }
    }

    const int choice = story_ai_choose_values(arguments);
    if (choice < 0) throw std::runtime_error("AI.choose failed");
    return Value::numberValue(choice);
}

StoryRuntime::Value StoryRuntime::evaluateBinary(const BinaryExpr &expr) const
{
    const Value left = evaluate(*expr.left);
    const Value right = evaluate(*expr.right);

    if (expr.op == "==") return Value::booleanValue(sameValue(left, right));
    if (expr.op == "!=") return Value::booleanValue(!sameValue(left, right));

    if (expr.op == "&&") return Value::booleanValue(truthy(left) && truthy(right));
    if (expr.op == "||") return Value::booleanValue(truthy(left) || truthy(right));

    if (left.kind != Value::Kind::Number || right.kind != Value::Kind::Number)
    {
        throw std::runtime_error("Numeric comparison requires numeric values");
    }

    if (expr.op == "+") return Value::numberValue(left.number + right.number);
    if (expr.op == "-") return Value::numberValue(left.number - right.number);
    if (expr.op == "*") return Value::numberValue(left.number * right.number);
    if (expr.op == "/") return Value::numberValue(left.number / right.number);
    if (expr.op == "%") return Value::numberValue(left.number % right.number);
    if (expr.op == "<") return Value::booleanValue(left.number < right.number);
    if (expr.op == "<=") return Value::booleanValue(left.number <= right.number);
    if (expr.op == ">") return Value::booleanValue(left.number > right.number);
    if (expr.op == ">=") return Value::booleanValue(left.number >= right.number);

    throw std::runtime_error("Unsupported operator: " + expr.op);
}

StoryRuntime::Value StoryRuntime::evaluateUnary(const UnaryExpr &expr) const
{
    const Value value = evaluate(*expr.operand);
    if (expr.op == "!") return Value::booleanValue(!truthy(value));
    if (expr.op == "-" && value.kind == Value::Kind::Number) return Value::numberValue(-value.number);
    throw std::runtime_error("Unsupported unary expression");
}

bool StoryRuntime::truthy(const Value &value)
{
    if (value.kind == Value::Kind::Boolean) return value.boolean;
    if (value.kind == Value::Kind::Number) return value.number != 0;
    if (value.kind == Value::Kind::String) return !value.text.empty();
    return false;
}

std::string StoryRuntime::asText(const Value &value)
{
    if (value.kind == Value::Kind::String) return value.text;
    if (value.kind == Value::Kind::Number) return std::to_string(value.number);
    if (value.kind == Value::Kind::Boolean) return value.boolean ? "true" : "false";
    return "";
}

bool StoryRuntime::sameValue(const Value &left, const Value &right)
{
    if (left.kind == Value::Kind::Number && right.kind == Value::Kind::Number)
    {
        return left.number == right.number;
    }
    return asText(left) == asText(right);
}

void StoryRuntime::execute(const Statement &statement)
{
    switch (statement.kind)
    {
    case NodeKind::Assignment:
    {
        const auto &assignment = static_cast<const AssignmentStmt &>(statement);
        values_[assignment.lvalue] = evaluate(*assignment.value);
        return;
    }
    case NodeKind::Action:
    {
        const auto &action = static_cast<const ActionStmt &>(statement);
        std::vector<std::string> arguments;
        for (const auto &argument : action.args) arguments.push_back(asText(evaluate(*argument)));

        if (action.name == "AI.generate")
        {
            story_ai_generate_values(arguments);
            return;
        }

        std::cout << "[action] " << action.name;
        if (!arguments.empty())
        {
            std::cout << "(";
            for (std::size_t i = 0; i < arguments.size(); ++i)
            {
                if (i != 0) std::cout << ", ";
                std::cout << arguments[i];
            }
            std::cout << ")";
        }
        std::cout << '\n';
        return;
    }
    case NodeKind::Branch:
    {
        const auto &branch = static_cast<const BranchStmt &>(statement);
        if (truthy(evaluate(*branch.condition))) executeBlock(branch.thenBlock);
        else executeBlock(branch.elseBlock);
        return;
    }
    case NodeKind::Loop:
    {
        const auto &loop = static_cast<const LoopStmt &>(statement);
        int iterations = 0;
        while (truthy(evaluate(*loop.condition)))
        {
            executeBlock(loop.body);
            if (++iterations > 10000) throw std::runtime_error("Loop exceeded runtime limit");
        }
        return;
    }
    case NodeKind::Return:
        return;
    default:
        throw std::runtime_error("Unsupported statement in runtime");
    }
}

void StoryRuntime::executeBlock(const std::vector<std::unique_ptr<Statement>> &statements)
{
    for (const auto &statement : statements) execute(*statement);
}
