#pragma once

#include "ast.h"

#include <map>

class StoryRuntime
{
public:
    void run(const Program &program);

private:
    struct Value
    {
        enum class Kind { Unknown, Number, String, Boolean } kind = Kind::Unknown;
        int number = 0;
        bool boolean = false;
        std::string text;

        static Value numberValue(int value);
        static Value stringValue(std::string value);
        static Value booleanValue(bool value);
    };

    std::map<std::string, Value> values_;
    std::map<std::string, int> enumValues_;

    Value evaluate(const Expr &expr) const;
    Value evaluateCall(const CallExpr &call) const;
    Value evaluateBinary(const BinaryExpr &expr) const;
    Value evaluateUnary(const UnaryExpr &expr) const;
    static bool truthy(const Value &value);
    static std::string asText(const Value &value);
    static bool sameValue(const Value &left, const Value &right);

    void execute(const Statement &statement);
    void executeBlock(const std::vector<std::unique_ptr<Statement>> &statements);
};
