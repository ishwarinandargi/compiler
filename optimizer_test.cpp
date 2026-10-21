#include "optimizer.h"
#include "codegen.h"
#include "parser.h"
#include <cassert>
#include <iostream>

namespace
{
    ThreeAddressCode generate3AC(const std::string &source)
    {
        Lexer lexer(source);
        Parser parser(lexer.tokenize());
        auto program = parser.parse();
        CodeGenerator codegen;
        codegen.generate(*program);
        return codegen.code();
    }

    void test_constant_folding()
    {
        std::cout << "\n--- Test 1: Constant Folding on Arithmetic ---" << std::endl;
        ThreeAddressCode ir;
        // Emit 3AC: t0 = 10 * 5, t1 = t0 + 20
        ir.emitBinary("*", "t0", "10", "5");
        ir.emitBinary("+", "t1", "t0", "20");
        ir.emitParam("t1");
        ir.emitAction("print", 1);

        Optimizer opt;
        auto optIR = opt.optimize(ir);

        std::cout << "Original 3AC:\n";
        ir.print(std::cout);
        std::cout << "\nOptimized 3AC:\n";
        optIR.print(std::cout);
        opt.printReport(std::cout);

        // Verify t0 was folded into 50 and propagated into param 70
        bool logFolded50 = false;
        bool logFolded70 = false;
        for (const auto &log : opt.logs())
        {
            if (log.pass == "Constant Folding" && log.after.find("50") != std::string::npos) logFolded50 = true;
            if (log.pass == "Constant Folding" && log.after.find("70") != std::string::npos) logFolded70 = true;
        }

        bool foundParam70 = false;
        for (const auto &q : optIR.quadruples())
        {
            if (q.op == "PARAM" && q.arg1 == "70") foundParam70 = true;
        }

        assert(logFolded50 && logFolded70 && foundParam70);
        std::cout << "PASS: Constant Folding Test\n";
    }

    void test_algebraic_simplification()
    {
        std::cout << "\n--- Test 2: Algebraic Simplification Identities ---" << std::endl;
        ThreeAddressCode ir;
        ir.emitBinary("+", "t0", "x", "0");
        ir.emitBinary("*", "t1", "y", "1");
        ir.emitBinary("*", "t2", "z", "0");
        ir.emitParam("t0");
        ir.emitParam("t1");
        ir.emitParam("t2");
        ir.emitAction("check", 3);

        Optimizer opt;
        auto optIR = opt.optimize(ir);

        std::cout << "\nOptimized 3AC for Identities:\n";
        optIR.print(std::cout);
        opt.printReport(std::cout);

        bool foundX = false;
        bool foundY = false;
        bool foundZero = false;

        for (const auto &q : optIR.quadruples())
        {
            if (q.result == "t0" && q.arg1 == "x") foundX = true;
            if (q.result == "t1" && q.arg1 == "y") foundY = true;
            if (q.result == "t2" && q.arg1 == "0") foundZero = true;
        }

        assert(foundX && foundY && foundZero);
        std::cout << "PASS: Algebraic Simplification Test\n";
    }

    void test_dead_code_elimination()
    {
        std::cout << "\n--- Test 3: Dead Code Elimination ---" << std::endl;
        ThreeAddressCode ir;
        ir.emitBinary("+", "t0", "a", "b"); // unused dead temporary
        ir.emitBinary("*", "t1", "c", "d"); // used in action
        ir.emitParam("t1");
        ir.emitAction("display", 1);

        Optimizer opt;
        auto optIR = opt.optimize(ir);

        std::cout << "\nOptimized 3AC:\n";
        optIR.print(std::cout);
        opt.printReport(std::cout);

        bool foundDeadT0 = false;
        bool foundUsedT1 = false;
        for (const auto &q : optIR.quadruples())
        {
            if (q.result == "t0") foundDeadT0 = true;
            if (q.result == "t1") foundUsedT1 = true;
        }

        assert(!foundDeadT0); // t0 should be eliminated
        assert(foundUsedT1);  // t1 should remain
        std::cout << "PASS: Dead Code Elimination Test\n";
    }
} // namespace

int main()
{
    try
    {
        std::cout << "========================================" << std::endl;
        std::cout << "  STORYLANG CODE OPTIMIZER TEST SUITE   " << std::endl;
        std::cout << "  STAGE 5: INTERMEDIATE CODE OPTIMIZER  " << std::endl;
        std::cout << "========================================" << std::endl;

        test_constant_folding();
        test_algebraic_simplification();
        test_dead_code_elimination();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  ALL OPTIMIZER TESTS PASSED!           " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "OPTIMIZER TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}
