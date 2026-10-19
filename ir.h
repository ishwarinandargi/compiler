#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

/**
 * Three-Address Code (3AC) Quadruple representation
 * Standard format taught in compiler design courses:
 * (operator, argument 1, argument 2, result)
 */
struct Quadruple
{
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    Quadruple(std::string op, std::string arg1 = "", std::string arg2 = "", std::string result = "")
        : op(std::move(op)), arg1(std::move(arg1)), arg2(std::move(arg2)), result(std::move(result)) {}

    // Formats quadruple as standard readable 3AC instruction (e.g., "t0 = a + b")
    std::string toInstructionString() const;

    // Formats as textbook quadruple tuple: (op, arg1, arg2, result)
    std::string toQuadrupleTuple() const;
};

/**
 * Container and manager for generated Three-Address Code
 */
class ThreeAddressCode
{
public:
    ThreeAddressCode() = default;

    // Emit a generic quadruple
    void emit(const std::string &op, const std::string &arg1 = "", const std::string &arg2 = "", const std::string &result = "");

    // Helper emit methods for classic 3AC
    void emitAssignment(const std::string &dest, const std::string &src);
    void emitBinary(const std::string &op, const std::string &dest, const std::string &arg1, const std::string &arg2);
    void emitUnary(const std::string &op, const std::string &dest, const std::string &arg);
    void emitLabel(const std::string &label);
    void emitGoto(const std::string &label);
    void emitIfFalse(const std::string &condition, const std::string &label);
    void emitIfTrue(const std::string &condition, const std::string &label);
    void emitParam(const std::string &param);
    void emitCall(const std::string &funcName, int argCount, const std::string &dest = "");
    void emitAction(const std::string &actionName, int argCount);
    void emitReturn(const std::string &val = "");

    const std::vector<Quadruple> &quadruples() const { return quadruples_; }

    // Print readable 3AC code listing
    void print(std::ostream &out = std::cout) const;

    // Print textbook Quadruple Table
    void printQuadrupleTable(std::ostream &out = std::cout) const;

    // Return the readable 3AC as a string
    std::string toString() const;

    // Clear all emitted code
    void clear() { quadruples_.clear(); }

private:
    std::vector<Quadruple> quadruples_;
};
