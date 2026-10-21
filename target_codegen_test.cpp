#include "target_codegen.h"
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

    void test_assembly_generation_arithmetic()
    {
        std::cout << "\n--- Test 1: Target Assembly for Arithmetic & Assignment ---" << std::endl;
        std::string source = "x := a + b * c";
        auto ir = generate3AC(source);

        TargetCodeGenerator targetGen;
        targetGen.generate(ir);

        std::string asmCode = targetGen.toAssemblyString();
        std::cout << "Generated x86-64 Assembly:\n" << asmCode << std::endl;
        targetGen.printAllocationTable(std::cout);
        targetGen.printTargetSummary(std::cout);

        assert(asmCode.find("pushl") != std::string::npos);
        assert(asmCode.find("movl") != std::string::npos);
        assert(asmCode.find("imull") != std::string::npos);
        assert(asmCode.find("addl") != std::string::npos);
        assert(asmCode.find("ret") != std::string::npos);

        std::cout << "PASS: Arithmetic & Assignment Assembly Generation Test\n";
    }

    void test_assembly_generation_branching()
    {
        std::cout << "\n--- Test 2: Target Assembly for Branching & Actions ---" << std::endl;
        std::string source = 
            "CHAR hero: wanderer\n"
            "STATE door: {open | locked}\n"
            "INIT door := open\n"
            "BRANCH door == open: ACTION enter(hero) ELSE: ACTION wait(hero)\n";

        auto ir = generate3AC(source);
        TargetCodeGenerator targetGen;
        targetGen.generate(ir);

        std::string asmCode = targetGen.toAssemblyString();
        std::cout << "Generated x86-64 Assembly:\n" << asmCode << std::endl;
        targetGen.printAllocationTable(std::cout);
        targetGen.printTargetSummary(std::cout);

        assert(asmCode.find("cmpl") != std::string::npos);
        assert(asmCode.find("sete") != std::string::npos);
        assert(asmCode.find("je") != std::string::npos);
        assert(asmCode.find("puts") != std::string::npos);
        assert(asmCode.find("L1:") != std::string::npos);
        assert(asmCode.find("L2:") != std::string::npos);

        std::cout << "PASS: Branching & Action Assembly Generation Test\n";
    }

    void test_assembly_generation_loop()
    {
        std::cout << "\n--- Test 3: Target Assembly for Loops (While Construct) ---" << std::endl;
        std::string source = "LOOP counter < 5: counter := counter + 1";

        auto ir = generate3AC(source);
        TargetCodeGenerator targetGen;
        targetGen.generate(ir);

        std::string asmCode = targetGen.toAssemblyString();
        std::cout << "Generated x86-64 Assembly:\n" << asmCode << std::endl;

        assert(asmCode.find("cmpl") != std::string::npos);
        assert(asmCode.find("setl") != std::string::npos);
        assert(asmCode.find("jmp") != std::string::npos);

        std::cout << "PASS: Loop Assembly Generation Test\n";
    }
} // namespace

int main()
{
    try
    {
        std::cout << "============================================" << std::endl;
        std::cout << "  STORYLANG TARGET CODE GENERATOR TEST     " << std::endl;
        std::cout << "  STAGE 6: TARGET MACHINE (x86-64) CODEGEN " << std::endl;
        std::cout << "============================================" << std::endl;

        test_assembly_generation_arithmetic();
        test_assembly_generation_branching();
        test_assembly_generation_loop();

        std::cout << "\n============================================" << std::endl;
        std::cout << "  ALL TARGET CODE GENERATOR TESTS PASSED!   " << std::endl;
        std::cout << "============================================" << std::endl;
        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "TARGET CODEGEN TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}
