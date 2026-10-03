#include "analyzer.h"
#include "parser.h"

#include <iostream>
#include <sstream>

namespace
{
void requireNoErrors(const std::string &input, const std::string &label)
{
    Lexer lexer(input);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse();
    SemanticAnalyzer analyzer;
    analyzer.analyze(*ast);
    if (!analyzer.errors().empty())
    {
        std::cerr << label << " unexpectedly failed: " << std::endl;
        for (const auto &err : analyzer.errors())
        {
            std::cerr << "  - " << err << std::endl;
        }
        throw std::runtime_error(label);
    }
}

void requireErrors(const std::string &input, const std::string &label)
{
    Lexer lexer(input);
    Parser parser(lexer.tokenize());
    auto ast = parser.parse();
    SemanticAnalyzer analyzer;
    analyzer.analyze(*ast);
    if (analyzer.errors().empty())
    {
        throw std::runtime_error(label + " should have failed");
    }
}
} // namespace

int main()
{
    try
    {
        requireNoErrors("CHAR alice: wanderer", "valid char declaration");

        requireErrors("CHAR alice: wanderer\nACTION walk(alice, store)", "undefined reference in action args");

        requireErrors("CHAR alice: wanderer\nCHAR alice: guardian", "duplicate declaration");

        requireNoErrors("CHAR alice: wanderer\nSTATE door: {open | locked}\nINIT door := open", "valid state assignment");

        requireErrors("STATE door: {open | locked}\nINIT door := closed", "invalid enum assignment");

        requireErrors("BRANCH \"hello\": RETURN 1", "invalid branch condition");

        requireNoErrors("CHAR alice: wanderer\nSTATE door: {open | locked}\nBRANCH door == open: ACTION walk(alice)", "valid comparison condition");

        requireNoErrors("FUNC greet(name): RETURN name", "valid function declaration");

        std::cout << "ALL ANALYZER TESTS PASSED" << std::endl;
        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "ANALYZER TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}
