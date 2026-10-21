#include "optimizer.h"
#include <iomanip>
#include <algorithm>
#include <unordered_set>

bool Optimizer::isInteger(const std::string &s) const
{
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0] == '-' || s[0] == '+')
    {
        if (s.length() == 1) return false;
        i = 1;
    }
    for (; i < s.length(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

long long Optimizer::parseInteger(const std::string &s) const
{
    return std::stoll(s);
}

void Optimizer::reset()
{
    logs_.clear();
    initialCount_ = 0;
    optimizedCount_ = 0;
}

double Optimizer::reductionPercentage() const
{
    if (initialCount_ == 0) return 0.0;
    return (1.0 - static_cast<double>(optimizedCount_) / initialCount_) * 100.0;
}

ThreeAddressCode Optimizer::constantFoldingAndPropagation(const ThreeAddressCode &input)
{
    ThreeAddressCode output;
    std::unordered_map<std::string, std::string> constMap;

    for (const auto &quad : input.quadruples())
    {
        Quadruple q = quad;

        // Invalidate constants on control flow boundaries
        if (q.op == "LABEL" || q.op == "GOTO" || q.op == "IFFALSE" || 
            q.op == "IFTRUE" || q.op == "FUNC_BEGIN" || q.op == "FUNC_END")
        {
            constMap.clear();
            output.emit(q.op, q.arg1, q.arg2, q.result);
            continue;
        }

        // Constant propagation: replace arguments with known constants
        if (!q.arg1.empty() && constMap.count(q.arg1))
        {
            std::string oldArg1 = q.arg1;
            q.arg1 = constMap[q.arg1];
            logs_.push_back({
                "Constant Propagation",
                "Replaced variable '" + oldArg1 + "' with constant value " + q.arg1,
                quad.toInstructionString(),
                q.toInstructionString()
            });
        }
        if (!q.arg2.empty() && constMap.count(q.arg2))
        {
            std::string oldArg2 = q.arg2;
            q.arg2 = constMap[q.arg2];
            logs_.push_back({
                "Constant Propagation",
                "Replaced variable '" + oldArg2 + "' with constant value " + q.arg2,
                quad.toInstructionString(),
                q.toInstructionString()
            });
        }

        // Constant folding for binary operators
        if (isInteger(q.arg1) && isInteger(q.arg2))
        {
            long long v1 = parseInteger(q.arg1);
            long long v2 = parseInteger(q.arg2);
            bool folded = false;
            long long resultVal = 0;

            if (q.op == "+") { resultVal = v1 + v2; folded = true; }
            else if (q.op == "-") { resultVal = v1 - v2; folded = true; }
            else if (q.op == "*") { resultVal = v1 * v2; folded = true; }
            else if (q.op == "/" && v2 != 0) { resultVal = v1 / v2; folded = true; }
            else if (q.op == "%" && v2 != 0) { resultVal = v1 % v2; folded = true; }
            else if (q.op == "==") { resultVal = (v1 == v2) ? 1 : 0; folded = true; }
            else if (q.op == "!=") { resultVal = (v1 != v2) ? 1 : 0; folded = true; }
            else if (q.op == "<") { resultVal = (v1 < v2) ? 1 : 0; folded = true; }
            else if (q.op == "<=") { resultVal = (v1 <= v2) ? 1 : 0; folded = true; }
            else if (q.op == ">") { resultVal = (v1 > v2) ? 1 : 0; folded = true; }
            else if (q.op == ">=") { resultVal = (v1 >= v2) ? 1 : 0; folded = true; }

            if (folded)
            {
                std::string resStr = std::to_string(resultVal);
                logs_.push_back({
                    "Constant Folding",
                    "Folded expression (" + q.arg1 + " " + q.op + " " + q.arg2 + ") into " + resStr,
                    q.toInstructionString(),
                    q.result + " = " + resStr
                });

                output.emitAssignment(q.result, resStr);
                constMap[q.result] = resStr;
                continue;
            }
        }
        else if (q.op == "NEG" && isInteger(q.arg1))
        {
            long long v1 = parseInteger(q.arg1);
            std::string resStr = std::to_string(-v1);
            logs_.push_back({
                "Constant Folding",
                "Folded unary negation (-" + q.arg1 + ") into " + resStr,
                q.toInstructionString(),
                q.result + " = " + resStr
            });
            output.emitAssignment(q.result, resStr);
            constMap[q.result] = resStr;
            continue;
        }

        // Track plain constant assignments
        if (q.op == "=")
        {
            if (isInteger(q.arg1))
            {
                constMap[q.result] = q.arg1;
            }
            else
            {
                constMap.erase(q.result);
            }
        }
        else if (!q.result.empty())
        {
            constMap.erase(q.result);
        }

        output.emit(q.op, q.arg1, q.arg2, q.result);
    }

    return output;
}

ThreeAddressCode Optimizer::algebraicSimplification(const ThreeAddressCode &input)
{
    ThreeAddressCode output;

    for (const auto &q : input.quadruples())
    {
        if (q.op == "+" && q.arg2 == "0")
        {
            // x + 0 => x
            logs_.push_back({"Algebraic Simplification", "Replaced (x + 0) with x", q.toInstructionString(), q.result + " = " + q.arg1});
            output.emitAssignment(q.result, q.arg1);
        }
        else if (q.op == "+" && q.arg1 == "0")
        {
            // 0 + x => x
            logs_.push_back({"Algebraic Simplification", "Replaced (0 + x) with x", q.toInstructionString(), q.result + " = " + q.arg2});
            output.emitAssignment(q.result, q.arg2);
        }
        else if (q.op == "-" && q.arg2 == "0")
        {
            // x - 0 => x
            logs_.push_back({"Algebraic Simplification", "Replaced (x - 0) with x", q.toInstructionString(), q.result + " = " + q.arg1});
            output.emitAssignment(q.result, q.arg1);
        }
        else if (q.op == "*" && (q.arg2 == "1" || q.arg1 == "1"))
        {
            // x * 1 => x or 1 * x => x
            std::string var = (q.arg2 == "1") ? q.arg1 : q.arg2;
            logs_.push_back({"Algebraic Simplification", "Replaced (x * 1) with x", q.toInstructionString(), q.result + " = " + var});
            output.emitAssignment(q.result, var);
        }
        else if (q.op == "*" && (q.arg2 == "0" || q.arg1 == "0"))
        {
            // x * 0 => 0 or 0 * x => 0
            logs_.push_back({"Algebraic Simplification", "Replaced (x * 0) with 0", q.toInstructionString(), q.result + " = 0"});
            output.emitAssignment(q.result, "0");
        }
        else if (q.op == "/" && q.arg2 == "1")
        {
            // x / 1 => x
            logs_.push_back({"Algebraic Simplification", "Replaced (x / 1) with x", q.toInstructionString(), q.result + " = " + q.arg1});
            output.emitAssignment(q.result, q.arg1);
        }
        else
        {
            output.emit(q.op, q.arg1, q.arg2, q.result);
        }
    }

    return output;
}

ThreeAddressCode Optimizer::deadCodeElimination(const ThreeAddressCode &input)
{
    const auto &quads = input.quadruples();
    std::unordered_set<std::string> usedTemps;

    // Collect all referenced arguments/conditions
    for (const auto &q : quads)
    {
        if (!q.arg1.empty() && q.arg1[0] == 't') usedTemps.insert(q.arg1);
        if (!q.arg2.empty() && q.arg2[0] == 't') usedTemps.insert(q.arg2);
        if ((q.op == "PARAM" || q.op == "RETURN" || q.op == "IFFALSE" || q.op == "IFTRUE") && 
            !q.result.empty() && q.result[0] == 't')
        {
            usedTemps.insert(q.result);
        }
    }

    ThreeAddressCode output;
    for (const auto &q : quads)
    {
        // Check if this is an assignment to an unused compiler temporary
        if ((q.op == "=" || q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/" || 
             q.op == "%" || q.op == "==" || q.op == "!=" || q.op == "<" || q.op == "<=" || 
             q.op == ">" || q.op == ">=") &&
            !q.result.empty() && q.result[0] == 't' && !usedTemps.count(q.result))
        {
            logs_.push_back({
                "Dead Code Elimination",
                "Eliminated unused computation stored in temporary '" + q.result + "'",
                q.toInstructionString(),
                "[Removed]"
            });
            continue; // Skip emitting dead code
        }

        output.emit(q.op, q.arg1, q.arg2, q.result);
    }

    return output;
}

ThreeAddressCode Optimizer::optimize(const ThreeAddressCode &input)
{
    reset();
    initialCount_ = static_cast<int>(input.quadruples().size());

    ThreeAddressCode current = input;
    // Run optimization passes
    current = constantFoldingAndPropagation(current);
    current = algebraicSimplification(current);
    current = deadCodeElimination(current);

    optimizedCount_ = static_cast<int>(current.quadruples().size());
    return current;
}

void Optimizer::printReport(std::ostream &out) const
{
    out << "\n+--------------------------------------------------------------------------+\n";
    out << "|                     CODE OPTIMIZATION SUMMARY REPORT                     |\n";
    out << "+--------------------------------------------------------------------------+\n";
    out << "| Initial 3AC Quadruples  : " << std::setw(46) << std::left << initialCount_ << " |\n";
    out << "| Optimized 3AC Quadruples: " << std::setw(46) << std::left << optimizedCount_ << " |\n";
    out << "| Total Transformations   : " << std::setw(46) << std::left << logs_.size() << " |\n";
    out << "+--------------------------------------------------------------------------+\n";

    if (logs_.empty())
    {
        out << "| Result: Code is already in optimal canonical form.                       |\n";
        out << "+--------------------------------------------------------------------------+\n";
        return;
    }

    out << "\n--- Optimization Log Table ---\n";
    out << "+----+--------------------------+---------------------------------+---------------------------------+\n";
    out << "| #  | Pass Name                | Before Optimization             | After Optimization              |\n";
    out << "+----+--------------------------+---------------------------------+---------------------------------+\n";

    int index = 1;
    for (const auto &log : logs_)
    {
        std::string before = log.before.length() > 31 ? log.before.substr(0, 28) + "..." : log.before;
        std::string after = log.after.length() > 31 ? log.after.substr(0, 28) + "..." : log.after;

        out << "| " << std::setw(2) << std::right << index++ << " | "
            << std::setw(24) << std::left << log.pass << " | "
            << std::setw(31) << std::left << before << " | "
            << std::setw(31) << std::left << after << " |\n";
    }
    out << "+----+--------------------------+---------------------------------+---------------------------------+\n";
}
