#pragma once

#include "ir.h"
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

/**
 * Optimization report entry documenting each transformation performed
 */
struct OptimizationLog
{
    std::string pass;
    std::string description;
    std::string before;
    std::string after;
};

/**
 * Machine-Independent Code Optimizer (Stage 5 of the Compiler)
 * Performs intermediate code optimizations on Three-Address Code (3AC):
 *   1. Constant Folding (evaluates compile-time constant arithmetic & logic)
 *   2. Constant & Copy Propagation (propagates known constants & aliases)
 *   3. Algebraic Simplification (identities: x+0, x*1, x*0, etc.)
 *   4. Dead Code Elimination (removes unused temporary assignments)
 */
class Optimizer
{
public:
    Optimizer() = default;

    // Run all optimization passes until fixpoint (or max passes)
    ThreeAddressCode optimize(const ThreeAddressCode &input);

    // Individual optimization passes
    ThreeAddressCode constantFoldingAndPropagation(const ThreeAddressCode &input);
    ThreeAddressCode algebraicSimplification(const ThreeAddressCode &input);
    ThreeAddressCode deadCodeElimination(const ThreeAddressCode &input);

    // Access optimization logs and statistics
    const std::vector<OptimizationLog> &logs() const { return logs_; }
    int initialInstructionCount() const { return initialCount_; }
    int optimizedInstructionCount() const { return optimizedCount_; }
    double reductionPercentage() const;

    // Print optimization summary report
    void printReport(std::ostream &out = std::cout) const;

    // Reset optimizer state
    void reset();

private:
    std::vector<OptimizationLog> logs_;
    int initialCount_ = 0;
    int optimizedCount_ = 0;

    bool isInteger(const std::string &s) const;
    long long parseInteger(const std::string &s) const;
};
