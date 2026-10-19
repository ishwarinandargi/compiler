#include "codegen.h"
#include "parser.h"
#include <cassert>
#include <iostream>
#include <sstream>

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

    void test_arithmetic_expressions()
    {
        std::cout << "\n--- Test 1: Arithmetic Expressions ---" << std::endl;
        std::string source = "x := a + b * c";
        auto ir = generate3AC(source);

        ir.print(std::cout);
        ir.printQuadrupleTable(std::cout);

        const auto &quads = ir.quadruples();
        assert(!quads.empty());

        // Verify multiplication comes before addition
        bool foundMul = false;
        bool foundAdd = false;
        for (const auto &q : quads)
        {
            if (q.op == "*")
            {
                foundMul = true;
                assert(q.arg1 == "b" && q.arg2 == "c");
            }
            if (q.op == "+")
            {
                foundAdd = true;
                assert(foundMul); // * evaluated before +
            }
        }
        assert(foundMul && foundAdd);
        std::cout << "PASS: Arithmetic expressions 3AC test\n";
    }

    void test_branch_statement()
    {
        std::cout << "\n--- Test 2: Branch Statement (If-Else) ---" << std::endl;
        std::string source = "BRANCH health > 0: ACTION play(hero) ELSE: ACTION game_over(hero)";
        auto ir = generate3AC(source);

        ir.print(std::cout);
        ir.printQuadrupleTable(std::cout);

        const auto &quads = ir.quadruples();
        bool foundCmp = false;
        bool foundIfFalse = false;
        bool foundGoto = false;
        bool foundLabel = false;

        for (const auto &q : quads)
        {
            if (q.op == ">") foundCmp = true;
            if (q.op == "IFFALSE") foundIfFalse = true;
            if (q.op == "GOTO") foundGoto = true;
            if (q.op == "LABEL") foundLabel = true;
        }

        assert(foundCmp && foundIfFalse && foundGoto && foundLabel);
        std::cout << "PASS: Branch statement 3AC test\n";
    }

    void test_loop_statement()
    {
        std::cout << "\n--- Test 3: Loop Statement (While) ---" << std::endl;
        std::string source = "LOOP counter < 10: counter := counter + 1";
        auto ir = generate3AC(source);

        ir.print(std::cout);
        ir.printQuadrupleTable(std::cout);

        const auto &quads = ir.quadruples();
        int labelCount = 0;
        bool foundIfFalse = false;
        bool foundGoto = false;

        for (const auto &q : quads)
        {
            if (q.op == "LABEL") labelCount++;
            if (q.op == "IFFALSE") foundIfFalse = true;
            if (q.op == "GOTO") foundGoto = true;
        }

        assert(labelCount >= 2);
        assert(foundIfFalse && foundGoto);
        std::cout << "PASS: Loop statement 3AC test\n";
    }

    void test_function_declaration()
    {
        std::cout << "\n--- Test 4: Function with Parameters and Return ---" << std::endl;
        std::string source = "FUNC add(a, b): RETURN a + b";
        auto ir = generate3AC(source);

        ir.print(std::cout);
        ir.printQuadrupleTable(std::cout);

        const auto &quads = ir.quadruples();
        bool foundFunc = false;
        bool foundParamA = false;
        bool foundParamB = false;
        bool foundReturn = false;

        for (const auto &q : quads)
        {
            if (q.op == "FUNC_BEGIN" && q.arg1 == "add") foundFunc = true;
            if (q.op == "PARAM" && q.arg1 == "a") foundParamA = true;
            if (q.op == "PARAM" && q.arg1 == "b") foundParamB = true;
            if (q.op == "RETURN") foundReturn = true;
        }

        assert(foundFunc && foundParamA && foundParamB && foundReturn);
        std::cout << "PASS: Function declaration 3AC test\n";
    }

    void test_declarations_and_actions()
    {
        std::cout << "\n--- Test 5: Character & State Declarations with Action ---" << std::endl;
        std::string source = "CHAR hero: warrior\nSTATE status: {idle | active}\nACTION attack(hero, dragon)";
        auto ir = generate3AC(source);

        ir.print(std::cout);
        ir.printQuadrupleTable(std::cout);

        const auto &quads = ir.quadruples();
        bool foundChar = false;
        bool foundState = false;
        bool foundAction = false;

        for (const auto &q : quads)
        {
            if (q.op == "CHAR_DECL") foundChar = true;
            if (q.op == "STATE_DECL") foundState = true;
            if (q.op == "ACTION" && q.arg1 == "attack") foundAction = true;
        }

        assert(foundChar && foundState && foundAction);
        std::cout << "PASS: Declarations and Action 3AC test\n";
    }

    void test_full_storylang_sample()
    {
        std::cout << "\n--- Test 6: Full StoryLang Integration ---" << std::endl;
        std::string source =
            "CHAR hero: wanderer\n"
            "STATE door: {open | locked}\n"
            "INIT door := open\n"
            "BRANCH door == open: ACTION enter(hero) ELSE: ACTION wait(hero)\n";
        auto ir = generate3AC(source);

        std::cout << "\n=== Generated 3AC for Story Sample ===" << std::endl;
        ir.print(std::cout);

        std::cout << "\n=== Quadruple Table ===" << std::endl;
        ir.printQuadrupleTable(std::cout);

        assert(!ir.quadruples().empty());
        std::cout << "PASS: Full StoryLang sample 3AC test\n";
    }
} // namespace

int main()
{
    try
    {
        std::cout << "========================================" << std::endl;
        std::cout << "  STORYLANG THREE-ADDRESS CODE (3AC)   " << std::endl;
        std::cout << "  STAGE 4 INTERMEDIATE CODE GENERATION " << std::endl;
        std::cout << "========================================" << std::endl;

        test_arithmetic_expressions();
        test_branch_statement();
        test_loop_statement();
        test_function_declaration();
        test_declarations_and_actions();
        test_full_storylang_sample();

        std::cout << "\n========================================" << std::endl;
        std::cout << "  ALL 3AC CODEGEN TESTS PASSED!         " << std::endl;
        std::cout << "========================================" << std::endl;
        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "3AC TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}
