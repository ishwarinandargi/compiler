#pragma once

#include "ir.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>

/**
 * Storage description for a variable or intermediate temporary
 */
struct StorageLocation
{
    std::string name;
    std::string kind;     // "Variable", "Temporary", "State/Enum", "Character"
    int offset = 0;       // Stack offset relative to %ebp (e.g. -4, -8)
    int size = 4;         // Size in bytes (32-bit standard word)
    std::string reg;      // Register if currently allocated
};

/**
 * Assembly directive / instruction line with educational comments
 */
struct AsmInstruction
{
    std::string label;
    std::string opcode;
    std::string operands;
    std::string comment;

    std::string toString() const;
};

/**
 * Target Machine Code Generator (Final Stage of the Compiler)
 * Translates Three-Address Code (3AC) into standard x86 Machine Assembly
 * (GNU / MinGW GCC Compatible, standard college compiler curriculum target).
 *
 * Implements:
 *   - Symbol / Temporary Stack Frame Allocation (%ebp offsets)
 *   - Register Allocation (%eax, %ebx, %ecx, %edx, %esi, %edi)
 *   - Data Segment (.data / .rdata) generation for strings, states, and actions
 *   - Text Segment (.text) generation with full prologues/epilogues
 *   - Full mapping of 3AC arithmetic, relational, branching, and runtime actions
 */
class TargetCodeGenerator
{
public:
    TargetCodeGenerator() = default;

    // Generate Target Assembly from 3AC
    void generate(const ThreeAddressCode &ir);

    // Formatted output functions
    void printAssembly(std::ostream &out = std::cout) const;
    void printAllocationTable(std::ostream &out = std::cout) const;
    void printTargetSummary(std::ostream &out = std::cout) const;

    // Return the generated assembly text
    std::string toAssemblyString() const;

    // Accessors
    const std::vector<AsmInstruction> &instructions() const { return instructions_; }
    const std::map<std::string, StorageLocation> &storageMap() const { return storageMap_; }
    int totalStackAllocated() const { return currentStackOffset_; }

    void reset();

private:
    std::vector<AsmInstruction> instructions_;
    std::map<std::string, StorageLocation> storageMap_;
    std::vector<std::pair<std::string, std::string>> dataStrings_; // label -> string literal
    std::unordered_map<std::string, int> stateEnums_;             // enum string -> numeric value
    std::unordered_map<std::string, std::string> actionLabels_;   // action call signature -> label
    int currentStackOffset_ = 0;
    int dataLabelCount_ = 0;

    // Allocation helpers
    int allocateStorage(const std::string &name, const std::string &kind = "Variable");
    std::string getLocation(const std::string &name);
    std::string addDataString(const std::string &str, const std::string &prefix = ".LC_str");

    // Assembly emission helpers
    void emit(const std::string &opcode, const std::string &operands = "", const std::string &comment = "");
    void emitLabel(const std::string &label);
    void emitComment(const std::string &comment);

    bool isNumber(const std::string &s) const;
    void collectSymbols(const ThreeAddressCode &ir);
};
