#include "ir.h"
#include <sstream>

std::string Quadruple::toInstructionString() const
{
    if (op == "LABEL")
    {
        return result + ":";
    }
    if (op == "=")
    {
        return result + " = " + arg1;
    }
    if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" ||
        op == "==" || op == "!=" || op == "<" || op == "<=" || op == ">" || op == ">=" ||
        op == "&&" || op == "||")
    {
        return result + " = " + arg1 + " " + op + " " + arg2;
    }
    if (op == "NEG")
    {
        return result + " = -" + arg1;
    }
    if (op == "NOT")
    {
        return result + " = !" + arg1;
    }
    if (op == "GOTO")
    {
        return "goto " + result;
    }
    if (op == "IFFALSE")
    {
        return "if_false " + arg1 + " goto " + result;
    }
    if (op == "IFTRUE")
    {
        return "if " + arg1 + " goto " + result;
    }
    if (op == "PARAM")
    {
        return "param " + arg1;
    }
    if (op == "CALL")
    {
        if (result.empty())
        {
            return "call " + arg1 + ", " + arg2;
        }
        return result + " = call " + arg1 + ", " + arg2;
    }
    if (op == "ACTION")
    {
        return "action " + arg1 + " (" + arg2 + " args)";
    }
    if (op == "RETURN")
    {
        if (arg1.empty())
        {
            return "return";
        }
        return "return " + arg1;
    }
    if (op == "CHAR_DECL")
    {
        return ".char " + arg1 + " : " + arg2;
    }
    if (op == "STATE_DECL")
    {
        return ".state " + arg1 + " = {" + arg2 + "}";
    }
    if (op == "FUNC_BEGIN")
    {
        return "func " + arg1 + ":";
    }
    if (op == "FUNC_END")
    {
        return "end func " + arg1;
    }
    if (op == "EVENT_BEGIN")
    {
        return "event " + arg1 + ":";
    }
    if (op == "EVENT_END")
    {
        return "end event " + arg1;
    }

    // Default fallback
    std::string s = op;
    if (!arg1.empty()) s += " " + arg1;
    if (!arg2.empty()) s += ", " + arg2;
    if (!result.empty()) s += " -> " + result;
    return s;
}

std::string Quadruple::toQuadrupleTuple() const
{
    return "(" + op + ", " +
           (arg1.empty() ? "-" : arg1) + ", " +
           (arg2.empty() ? "-" : arg2) + ", " +
           (result.empty() ? "-" : result) + ")";
}

void ThreeAddressCode::emit(const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &result)
{
    quadruples_.emplace_back(op, arg1, arg2, result);
}

void ThreeAddressCode::emitAssignment(const std::string &dest, const std::string &src)
{
    emit("=", src, "", dest);
}

void ThreeAddressCode::emitBinary(const std::string &op, const std::string &dest, const std::string &arg1, const std::string &arg2)
{
    emit(op, arg1, arg2, dest);
}

void ThreeAddressCode::emitUnary(const std::string &op, const std::string &dest, const std::string &arg)
{
    emit(op, arg, "", dest);
}

void ThreeAddressCode::emitLabel(const std::string &label)
{
    emit("LABEL", "", "", label);
}

void ThreeAddressCode::emitGoto(const std::string &label)
{
    emit("GOTO", "", "", label);
}

void ThreeAddressCode::emitIfFalse(const std::string &condition, const std::string &label)
{
    emit("IFFALSE", condition, "", label);
}

void ThreeAddressCode::emitIfTrue(const std::string &condition, const std::string &label)
{
    emit("IFTRUE", condition, "", label);
}

void ThreeAddressCode::emitParam(const std::string &param)
{
    emit("PARAM", param, "", "");
}

void ThreeAddressCode::emitCall(const std::string &funcName, int argCount, const std::string &dest)
{
    emit("CALL", funcName, std::to_string(argCount), dest);
}

void ThreeAddressCode::emitAction(const std::string &actionName, int argCount)
{
    emit("ACTION", actionName, std::to_string(argCount), "");
}

void ThreeAddressCode::emitReturn(const std::string &val)
{
    emit("RETURN", val, "", "");
}

void ThreeAddressCode::print(std::ostream &out) const
{
    for (const auto &q : quadruples_)
    {
        if (q.op == "LABEL" || q.op == "FUNC_BEGIN" || q.op == "EVENT_BEGIN")
        {
            out << q.toInstructionString() << "\n";
        }
        else
        {
            out << "    " << q.toInstructionString() << "\n";
        }
    }
}

std::string ThreeAddressCode::toString() const
{
    std::ostringstream oss;
    print(oss);
    return oss.str();
}

void ThreeAddressCode::printQuadrupleTable(std::ostream &out) const
{
    out << "+----+----------------+----------------+----------------+----------------+\n";
    out << "| #  | Operator       | Argument 1     | Argument 2     | Result         |\n";
    out << "+----+----------------+----------------+----------------+----------------+\n";

    for (size_t i = 0; i < quadruples_.size(); ++i)
    {
        const auto &q = quadruples_[i];
        out << "| " << std::setw(2) << std::left << i << " | "
            << std::setw(14) << std::left << q.op << " | "
            << std::setw(14) << std::left << (q.arg1.empty() ? "-" : q.arg1) << " | "
            << std::setw(14) << std::left << (q.arg2.empty() ? "-" : q.arg2) << " | "
            << std::setw(14) << std::left << (q.result.empty() ? "-" : q.result) << " |\n";
    }

    out << "+----+----------------+----------------+----------------+----------------+\n";
}
