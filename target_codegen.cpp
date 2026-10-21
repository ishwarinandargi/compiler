#include "target_codegen.h"
#include <iomanip>
#include <sstream>
#include <cctype>

std::string AsmInstruction::toString() const
{
    std::string line;
    if (!label.empty())
    {
        line += label + ":";
        if (opcode.empty() && comment.empty()) return line;
        line += "\n";
    }

    if (!opcode.empty())
    {
        line += "    " + opcode;
        if (!operands.empty())
        {
            if (opcode.length() < 8)
            {
                line += std::string(8 - opcode.length(), ' ') + operands;
            }
            else
            {
                line += " " + operands;
            }
        }
    }

    if (!comment.empty())
    {
        if (opcode.empty() && label.empty())
        {
            line = "    # " + comment;
        }
        else
        {
            size_t pad = (line.length() < 36) ? (36 - line.length()) : 2;
            line += std::string(pad, ' ') + "# " + comment;
        }
    }

    return line;
}

void TargetCodeGenerator::reset()
{
    instructions_.clear();
    storageMap_.clear();
    dataStrings_.clear();
    stateEnums_.clear();
    actionLabels_.clear();
    currentStackOffset_ = 0;
    dataLabelCount_ = 0;
}

bool TargetCodeGenerator::isNumber(const std::string &s) const
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

int TargetCodeGenerator::allocateStorage(const std::string &name, const std::string &kind)
{
    if (storageMap_.count(name))
    {
        return storageMap_[name].offset;
    }

    currentStackOffset_ += 4;
    StorageLocation loc;
    loc.name = name;
    loc.kind = kind;
    loc.offset = currentStackOffset_;
    loc.size = 4;
    storageMap_[name] = loc;
    return currentStackOffset_;
}

std::string TargetCodeGenerator::addDataString(const std::string &str, const std::string &prefix)
{
    std::string lbl = prefix + "_" + std::to_string(++dataLabelCount_);
    dataStrings_.push_back({lbl, str});
    return lbl;
}

std::string TargetCodeGenerator::getLocation(const std::string &name)
{
    if (name.empty()) return "";

    // Immediate integer constant
    if (isNumber(name))
    {
        return "$" + name;
    }

    // State enum value mapped to integer constant
    if (stateEnums_.count(name))
    {
        return "$" + std::to_string(stateEnums_[name]);
    }

    // String literal
    if (name.front() == '"' && name.back() == '"')
    {
        std::string unquoted = name.substr(1, name.length() - 2);
        std::string lbl = addDataString(unquoted, ".LC_str");
        return "$" + lbl;
    }

    // Stack variable or compiler temporary (e.g. -4(%ebp), -8(%ebp))
    std::string kind = (name[0] == 't') ? "Temporary" : "Variable";
    int offset = allocateStorage(name, kind);
    return "-" + std::to_string(offset) + "(%ebp)";
}

void TargetCodeGenerator::emit(const std::string &opcode, const std::string &operands, const std::string &comment)
{
    instructions_.push_back({"", opcode, operands, comment});
}

void TargetCodeGenerator::emitLabel(const std::string &label)
{
    instructions_.push_back({label, "", "", ""});
}

void TargetCodeGenerator::emitComment(const std::string &comment)
{
    instructions_.push_back({"", "", "", comment});
}

void TargetCodeGenerator::collectSymbols(const ThreeAddressCode &ir)
{
    std::vector<std::string> pendingParams;
    int actionIndex = 0;

    for (const auto &q : ir.quadruples())
    {
        if (q.op == "CHAR_DECL")
        {
            allocateStorage(q.arg1, "Character");
            addDataString(q.arg2, ".LC_char_" + q.arg1);
        }
        else if (q.op == "STATE_DECL")
        {
            allocateStorage(q.arg1, "State/Enum");
            std::stringstream ss(q.arg2);
            std::string item;
            int enumVal = 0;
            while (std::getline(ss, item, '|'))
            {
                size_t first = item.find_first_not_of(" \t");
                size_t last = item.find_last_not_of(" \t");
                if (first != std::string::npos && last != std::string::npos)
                {
                    std::string trimmed = item.substr(first, last - first + 1);
                    stateEnums_[trimmed] = enumVal++;
                }
            }
        }
        else if (q.op == "PARAM")
        {
            pendingParams.push_back(q.arg1);
        }
        else if (q.op == "ACTION")
        {
            std::string actionName = q.arg1;
            std::string msg = ">>> [ACTION EXECUTION] ACTION: " + actionName;
            if (!pendingParams.empty())
            {
                msg += "(";
                for (size_t i = 0; i < pendingParams.size(); ++i)
                {
                    msg += pendingParams[i];
                    if (i + 1 < pendingParams.size()) msg += ", ";
                }
                msg += ")";
            }
            std::string actLabel = addDataString(msg, ".LC_act_" + actionName);
            actionLabels_["act_" + std::to_string(actionIndex++)] = actLabel;
            pendingParams.clear();
        }
        else
        {
            pendingParams.clear();
            if (!q.result.empty() && q.op != "GOTO" && q.op != "IFFALSE" && q.op != "IFTRUE" && q.op != "LABEL")
            {
                std::string kind = (q.result[0] == 't') ? "Temporary" : "Variable";
                allocateStorage(q.result, kind);
            }
            if (!q.arg1.empty() && !isNumber(q.arg1) && q.arg1.front() != '"' && q.op != "FUNC_BEGIN" && q.op != "CALL" && q.op != "ACTION")
            {
                if (!stateEnums_.count(q.arg1))
                {
                    std::string kind = (q.arg1[0] == 't') ? "Temporary" : "Variable";
                    allocateStorage(q.arg1, kind);
                }
            }
            if (!q.arg2.empty() && !isNumber(q.arg2) && q.arg2.front() != '"' && q.op != "CALL" && q.op != "ACTION")
            {
                if (!stateEnums_.count(q.arg2))
                {
                    std::string kind = (q.arg2[0] == 't') ? "Temporary" : "Variable";
                    allocateStorage(q.arg2, kind);
                }
            }
        }
    }
}

void TargetCodeGenerator::generate(const ThreeAddressCode &ir)
{
    reset();
    collectSymbols(ir);

    // Compute 16-byte aligned stack allocation size
    int totalStack = ((currentStackOffset_ + 15) / 16) * 16;
    if (totalStack == 0) totalStack = 16;

    // Header comment
    emitComment("==========================================================================");
    emitComment("  STORYLANG COMPILER - TARGET CODE GENERATOR (x86 ASSEMBLY)               ");
    emitComment("  Target: x86 / IA-32 (GNU / MinGW GCC Compatible)                        ");
    emitComment("==========================================================================");

    // Read-only Data Section (.rdata)
    emitComment("--------------------------------------------------------------------------");
    emitComment("  DATA SEGMENT: String Literals, Action Messages & Program Constants      ");
    emitComment("--------------------------------------------------------------------------");
    emit(".section", ".rdata,\"dr\"");

    for (const auto &ds : dataStrings_)
    {
        emitLabel(ds.first);
        emit(".asciz", "\"" + ds.second + "\"");
    }

    // Text Section (.text)
    emitComment("--------------------------------------------------------------------------");
    emitComment("  TEXT SEGMENT: Executable Machine Instructions                           ");
    emitComment("--------------------------------------------------------------------------");
    emit(".text");

    std::vector<std::string> pendingParams;
    int actionIndex = 0;

    for (const auto &q : ir.quadruples())
    {
        if (q.op == "CHAR_DECL")
        {
            emitComment("Declaration: Character '" + q.arg1 + "' of role '" + q.arg2 + "'");
            continue;
        }

        if (q.op == "STATE_DECL")
        {
            emitComment("Declaration: State '" + q.arg1 + "' with values {" + q.arg2 + "}");
            continue;
        }

        if (q.op == "FUNC_BEGIN")
        {
            std::string funcName = q.arg1;

            emitComment("----------------------------------------------------------------------");
            emitComment("Function Definition: " + funcName);
            emitComment("----------------------------------------------------------------------");
            emit(".globl", "_" + funcName);
            emit(".globl", funcName);
            emit(".def", "_" + funcName + "; .scl 2; .type 32; .endef");
            emitLabel("_" + funcName);
            emitLabel(funcName);

            // Function Prologue
            emit("pushl", "%ebp", "Save old base pointer");
            emit("movl", "%esp, %ebp", "Establish new stack frame");
            emit("subl", "$" + std::to_string(totalStack) + ", %esp", "Allocate " + std::to_string(totalStack) + " bytes on stack");
            continue;
        }

        if (q.op == "FUNC_END")
        {
            // Function Epilogue
            emit("movl", "$0, %eax", "Return 0 exit code");
            emit("movl", "%ebp, %esp", "Deallocate stack frame");
            emit("popl", "%ebp", "Restore base pointer");
            emit("ret", "", "Return to caller");
            continue;
        }

        if (q.op == "LABEL")
        {
            emitLabel(q.result);
            continue;
        }

        if (q.op == "GOTO")
        {
            emit("jmp", q.result, "3AC: goto " + q.result);
            continue;
        }

        if (q.op == "IFFALSE")
        {
            std::string condLoc = getLocation(q.arg1);
            emit("movl", condLoc + ", %eax", "3AC: " + q.toInstructionString());
            emit("cmpl", "$0, %eax", "Test condition boolean value");
            emit("je", q.result, "Jump if false (zero)");
            continue;
        }

        if (q.op == "IFTRUE")
        {
            std::string condLoc = getLocation(q.arg1);
            emit("movl", condLoc + ", %eax", "3AC: " + q.toInstructionString());
            emit("cmpl", "$0, %eax", "Test condition boolean value");
            emit("jne", q.result, "Jump if true (non-zero)");
            continue;
        }

        if (q.op == "=")
        {
            std::string destLoc = getLocation(q.result);
            std::string srcLoc = getLocation(q.arg1);

            if (srcLoc.front() == '$')
            {
                emit("movl", srcLoc + ", " + destLoc, "3AC: " + q.result + " = " + q.arg1);
            }
            else
            {
                emit("movl", srcLoc + ", %eax", "3AC: " + q.result + " = " + q.arg1);
                emit("movl", "%eax, " + destLoc, "Store to destination");
            }
            continue;
        }

        if (q.op == "+" || q.op == "-" || q.op == "*" || q.op == "/" || q.op == "%")
        {
            std::string destLoc = getLocation(q.result);
            std::string arg1Loc = getLocation(q.arg1);
            std::string arg2Loc = getLocation(q.arg2);

            emit("movl", arg1Loc + ", %eax", "3AC: " + q.toInstructionString());

            if (q.op == "+")
            {
                emit("addl", arg2Loc + ", %eax", "Perform addition");
                emit("movl", "%eax, " + destLoc, "Store sum to " + q.result);
            }
            else if (q.op == "-")
            {
                emit("subl", arg2Loc + ", %eax", "Perform subtraction");
                emit("movl", "%eax, " + destLoc, "Store difference to " + q.result);
            }
            else if (q.op == "*")
            {
                emit("imull", arg2Loc + ", %eax", "Perform signed multiplication");
                emit("movl", "%eax, " + destLoc, "Store product to " + q.result);
            }
            else if (q.op == "/" || q.op == "%")
            {
                emit("cltd", "", "Sign-extend %eax into %edx:%eax");
                if (arg2Loc.front() == '$')
                {
                    emit("movl", arg2Loc + ", %ecx", "Load divisor into register");
                    emit("idivl", "%ecx", "Signed integer division");
                }
                else
                {
                    emit("idivl", arg2Loc, "Signed integer division");
                }

                if (q.op == "/")
                {
                    emit("movl", "%eax, " + destLoc, "Store quotient to " + q.result);
                }
                else
                {
                    emit("movl", "%edx, " + destLoc, "Store remainder to " + q.result);
                }
            }
            continue;
        }

        if (q.op == "==" || q.op == "!=" || q.op == "<" || q.op == "<=" || q.op == ">" || q.op == ">=")
        {
            std::string destLoc = getLocation(q.result);
            std::string arg1Loc = getLocation(q.arg1);
            std::string arg2Loc = getLocation(q.arg2);

            emit("movl", arg1Loc + ", %eax", "3AC: " + q.toInstructionString());
            emit("cmpl", arg2Loc + ", %eax", "Compare operands (" + q.arg1 + " vs " + q.arg2 + ")");

            std::string setInst;
            if (q.op == "==") setInst = "sete";
            else if (q.op == "!=") setInst = "setne";
            else if (q.op == "<") setInst = "setl";
            else if (q.op == "<=") setInst = "setle";
            else if (q.op == ">") setInst = "setg";
            else if (q.op == ">=") setInst = "setge";

            emit(setInst, "%al", "Set lower byte based on condition code");
            emit("movzbl", "%al, %eax", "Zero-extend %al into %eax");
            emit("movl", "%eax, " + destLoc, "Store boolean result to " + q.result);
            continue;
        }

        if (q.op == "NEG")
        {
            std::string destLoc = getLocation(q.result);
            std::string arg1Loc = getLocation(q.arg1);
            emit("movl", arg1Loc + ", %eax", "3AC: " + q.toInstructionString());
            emit("negl", "%eax", "Negate value");
            emit("movl", "%eax, " + destLoc, "Store to " + q.result);
            continue;
        }

        if (q.op == "NOT")
        {
            std::string destLoc = getLocation(q.result);
            std::string arg1Loc = getLocation(q.arg1);
            emit("movl", arg1Loc + ", %eax", "3AC: " + q.toInstructionString());
            emit("cmpl", "$0, %eax", "Compare with zero");
            emit("sete", "%al", "Set if zero (!0 == 1)");
            emit("movzbl", "%al, %eax", "Zero-extend");
            emit("movl", "%eax, " + destLoc, "Store to " + q.result);
            continue;
        }

        if (q.op == "PARAM")
        {
            pendingParams.push_back(q.arg1);
            emitComment("Parameter: " + q.arg1);
            continue;
        }

        if (q.op == "ACTION")
        {
            emitComment("3AC: " + q.toInstructionString());
            std::string key = "act_" + std::to_string(actionIndex++);
            std::string actLabel = actionLabels_[key];

            emit("pushl", "$" + actLabel, "Push action message address");
            emit("call", "_puts", "Trigger runtime action notification via puts");
            emit("addl", "$4, %esp", "Clean up stack");
            pendingParams.clear();
            continue;
        }

        if (q.op == "CALL")
        {
            std::string funcName = q.arg1;
            emitComment("3AC: " + q.toInstructionString());

            // Push parameters in reverse order (cdecl)
            for (auto it = pendingParams.rbegin(); it != pendingParams.rend(); ++it)
            {
                emit("pushl", getLocation(*it), "Push argument");
            }

            emit("call", "_" + funcName, "Call function " + funcName);
            if (!pendingParams.empty())
            {
                emit("addl", "$" + std::to_string(pendingParams.size() * 4) + ", %esp", "Clean argument stack");
            }
            if (!q.result.empty())
            {
                emit("movl", "%eax, " + getLocation(q.result), "Store returned result into " + q.result);
            }
            pendingParams.clear();
            continue;
        }

        if (q.op == "RETURN")
        {
            if (!q.arg1.empty())
            {
                emit("movl", getLocation(q.arg1) + ", %eax", "3AC: return " + q.arg1);
            }
            emit("movl", "%ebp, %esp", "Restore stack pointer");
            emit("popl", "%ebp", "Restore base pointer");
            emit("ret", "", "Return to caller");
            continue;
        }
    }
}

std::string TargetCodeGenerator::toAssemblyString() const
{
    std::stringstream ss;
    for (const auto &inst : instructions_)
    {
        ss << inst.toString() << "\n";
    }
    return ss.str();
}

void TargetCodeGenerator::printAssembly(std::ostream &out) const
{
    out << toAssemblyString();
}

void TargetCodeGenerator::printAllocationTable(std::ostream &out) const
{
    out << "+----------------------+----------------------+----------------------+----------------------+\n";
    out << "| Identifier / Temp    | Symbol Kind          | Stack Storage Offset | Size (Bytes)         |\n";
    out << "+----------------------+----------------------+----------------------+----------------------+\n";

    for (const auto &pair : storageMap_)
    {
        const auto &loc = pair.second;
        std::string offsetStr = "-" + std::to_string(loc.offset) + "(%ebp)";

        out << "| " << std::setw(20) << std::left << loc.name << " | "
            << std::setw(20) << std::left << loc.kind << " | "
            << std::setw(20) << std::left << offsetStr << " | "
            << std::setw(20) << std::left << loc.size << " |\n";
    }
    out << "+----------------------+----------------------+----------------------+----------------------+\n";
}

void TargetCodeGenerator::printTargetSummary(std::ostream &out) const
{
    int totalInst = 0;
    int labels = 0;
    for (const auto &inst : instructions_)
    {
        if (!inst.opcode.empty()) totalInst++;
        if (!inst.label.empty()) labels++;
    }

    out << "\n+--------------------------------------------------------------------------+\n";
    out << "|                    TARGET CODE GENERATION SUMMARY                        |\n";
    out << "+--------------------------------------------------------------------------+\n";
    out << "| Target Architecture     : x86 / IA-32 (GAS / MinGW GCC Compatible)      |\n";
    out << "| Total Assembly Lines    : " << std::setw(46) << std::left << instructions_.size() << " |\n";
    out << "| Executable Instructions : " << std::setw(46) << std::left << totalInst << " |\n";
    out << "| Jump Labels Defined     : " << std::setw(46) << std::left << labels << " |\n";
    out << "| Stack Memory Allocated  : " << std::setw(46) << std::left << (std::to_string(currentStackOffset_) + " bytes") << " |\n";
    out << "| Static Strings & Enums  : " << std::setw(46) << std::left << dataStrings_.size() << " |\n";
    out << "| Registers Allocated     : %eax, %ebx, %ecx, %edx, %ebp, %esp             |\n";
    out << "+--------------------------------------------------------------------------+\n";
}
