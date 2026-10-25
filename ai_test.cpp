#include "ai_runtime.h"
#include "analyzer.h"
#include "codegen.h"
#include "parser.h"
#include "runner.h"
#include "token.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

int main()
{
    _putenv("STORYLANG_AI_MOCK=1");

    const std::string source =
        "CHAR hero: wanderer\n"
        "STATE choice: {attack | flee}\n"
        "INIT choice := AI.choose(\"What next?\", \"attack\", \"flee\")\n"
        "ACTION AI.generate(\"Describe the scene\")\n";

    Lexer lexer(source);
    Parser parser(lexer.tokenize());
    auto program = parser.parse();

    SemanticAnalyzer analyzer;
    analyzer.analyze(*program);
    if (!analyzer.errors().empty())
    {
        throw std::runtime_error("AI source failed semantic analysis");
    }

    CodeGenerator generator;
    generator.generate(*program);
    const std::string ir = generator.code().toString();
    if (ir.find("call AI.choose") == std::string::npos ||
        ir.find("action AI.generate") == std::string::npos)
    {
        throw std::runtime_error("AI calls were not emitted into IR");
    }

    if (story_ai_choose(3, "What next?", "attack", "flee") != 0)
    {
        throw std::runtime_error("mock AI.choose did not select the first option");
    }

    StoryRuntime runtime;
    runtime.run(*program);

    std::cout << "AI integration test passed\n";
    return 0;
}
