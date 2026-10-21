#include "token.h"
#include "parser.h"
#include "ast.h"
#include "ir.h"
#include "codegen.h"
#include "optimizer.h"
#include "target_codegen.h"
#include "analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

/**
 * StoryLang Compiler CLI
 *
 * Usage:
 *   ./storylang_cli <input_file.slt>
 *   ./storylang_cli --stdin
 */

void printUsage(const std::string &programName)
{
    std::cout << "Usage: " << programName << " [OPTIONS] [FILE]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  --stdin              Read from standard input" << std::endl;
    std::cout << "  --verbose            Print detailed token information" << std::endl;
    std::cout << "  --output <file>      Write output to specified file" << std::endl;
    std::cout << "  --ast                Parse tokens and print the Abstract Syntax Tree" << std::endl;
    std::cout << "  --ir                 Generate and print Three-Address Code (3AC)" << std::endl;
    std::cout << "  --quads              Print 3AC Quadruple Table" << std::endl;
    std::cout << "  --opt                Run intermediate code optimizer and show report" << std::endl;
    std::cout << "  --target, --asm      Generate and print x86-64 Target Machine Assembly" << std::endl;
    std::cout << "  --all                Run and display all 6 compiler stages sequentially" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << programName << " demo.slt --all" << std::endl;
    std::cout << "  " << programName << " demo.slt --target" << std::endl;
    std::cout << "  " << programName << " demo.slt --ir --opt" << std::endl;
}

std::string escapeJsonString(const std::string &str)
{
    std::string result;
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\t':
            result += "\\t";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        default:
            if (c < 32)
            {
                result += "\\u" + std::string(4, '0');
            }
            else
            {
                result += c;
            }
        }
    }
    return result;
}

void printTokensAsJson(const std::vector<Token> &tokens)
{
    std::cout << "[" << std::endl;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        const auto &token = tokens[i];
        std::cout << "  {" << std::endl;
        std::cout << "    \"type\": \"" << escapeJsonString(token.typeName()) << "\"," << std::endl;
        std::cout << "    \"lexeme\": \"" << escapeJsonString(token.lexeme) << "\"," << std::endl;
        std::cout << "    \"line\": " << token.line << "," << std::endl;
        std::cout << "    \"column\": " << token.column << "," << std::endl;
        std::cout << "    \"position\": " << token.position << std::endl;

        if (!token.value.empty())
        {
            std::cout << "    \"value\": \"" << escapeJsonString(token.value) << "\"" << std::endl;
        }

        std::cout << "  }";
        if (i < tokens.size() - 1)
        {
            std::cout << ",";
        }
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

void printTokensAsTable(const std::vector<Token> &tokens)
{
    std::cout << "\n=== TOKEN STREAM ===" << std::endl;
    std::cout << std::setw(25) << std::left << "Token Type"
              << " | " << std::setw(20) << "Lexeme"
              << " | Location" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    for (const auto &token : tokens)
    {
        std::string lexemeDisplay = token.lexeme;
        if (lexemeDisplay.length() > 20)
        {
            lexemeDisplay = lexemeDisplay.substr(0, 17) + "...";
        }

        std::cout << std::setw(25) << std::left << token.typeName()
              << " | " << std::setw(20) << lexemeDisplay
              << " | L" << token.line << ":C" << token.column << std::endl;
    }

    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Total tokens: " << tokens.size() << std::endl
              << std::endl;
}

void printStatistics(const std::vector<Token> &tokens)
{
    std::cout << "\n=== STATISTICS ===" << std::endl;

    int keywordCount = 0;
    int identifierCount = 0;
    int numberCount = 0;
    int stringCount = 0;
    int operatorCount = 0;
    int delimiterCount = 0;

    for (const auto &token : tokens)
    {
        switch (token.type)
        {
        case TokenType::CHAR:
        case TokenType::STATE:
        case TokenType::EVENT:
        case TokenType::ACTION:
        case TokenType::BRANCH:
        case TokenType::ELSE:
        case TokenType::LOOP:
        case TokenType::FUNC:
        case TokenType::RETURN:
        case TokenType::INIT:
        case TokenType::PROP:
            keywordCount++;
            break;

        case TokenType::IDENTIFIER:
            identifierCount++;
            break;

        case TokenType::NUMBER:
            numberCount++;
            break;

        case TokenType::STRING:
            stringCount++;
            break;

        case TokenType::EQ:
        case TokenType::NE:
        case TokenType::LT:
        case TokenType::LE:
        case TokenType::GT:
        case TokenType::GE:
        case TokenType::ASSIGN:
        case TokenType::ARROW:
        case TokenType::PLUS:
        case TokenType::MINUS:
        case TokenType::STAR:
        case TokenType::SLASH:
        case TokenType::PERCENT:
        case TokenType::AND:
        case TokenType::OR:
        case TokenType::NOT:
            operatorCount++;
            break;

        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::LBRACE:
        case TokenType::RBRACE:
        case TokenType::LBRACKET:
        case TokenType::RBRACKET:
        case TokenType::COLON:
        case TokenType::SEMICOLON:
        case TokenType::COMMA:
        case TokenType::DOT:
        case TokenType::PIPE:
            delimiterCount++;
            break;

        default:
            break;
        }
    }

    std::cout << "  Keywords:   " << keywordCount << std::endl;
    std::cout << "  Identifiers: " << identifierCount << std::endl;
    std::cout << "  Numbers:    " << numberCount << std::endl;
    std::cout << "  Strings:    " << stringCount << std::endl;
    std::cout << "  Operators:  " << operatorCount << std::endl;
    std::cout << "  Delimiters: " << delimiterCount << std::endl;
    std::cout << "  Total:      " << tokens.size() << std::endl;
    std::cout << std::endl;
}

void printSymbolTable(const SymbolTable &symTable)
{
    std::cout << "+----------------+------------+----------------+--------------------------+\n";
    std::cout << "| Symbol Name    | Kind       | Data Type      | Attributes               |\n";
    std::cout << "+----------------+------------+----------------+--------------------------+\n";

    const auto &scopes = symTable.scopes();
    for (size_t i = 0; i < scopes.size(); ++i)
    {
        for (const auto &pair : scopes[i])
        {
            const auto &sym = pair.second;
            std::string kindStr;
            switch (sym.kind)
            {
            case Symbol::Kind::Char: kindStr = "Character"; break;
            case Symbol::Kind::State: kindStr = "State/Enum"; break;
            case Symbol::Kind::Func: kindStr = "Function"; break;
            case Symbol::Kind::Var: kindStr = "Variable"; break;
            }

            std::string attrStr;
            for (const auto &attr : sym.attributes)
            {
                attrStr += attr.first + "=" + attr.second + " ";
            }

            std::cout << "| " << std::setw(14) << std::left << sym.name << " | "
                      << std::setw(10) << std::left << kindStr << " | "
                      << std::setw(14) << std::left << (sym.dataType.empty() ? "-" : sym.dataType) << " | "
                      << std::setw(24) << std::left << (attrStr.empty() ? "-" : attrStr) << " |\n";
        }
    }
    std::cout << "+----------------+------------+----------------+--------------------------+\n";
}

int main(int argc, char *argv[])
{
    std::string inputFile;
    std::string outputFile;
    bool useStdin = false;
    bool verbose = false;
    bool printJson = false;
    bool printAst = false;
    bool printIR = false;
    bool printQuads = false;
    bool printOpt = false;
    bool printTarget = false;
    bool printAll = false;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help" || arg == "-h")
        {
            printUsage(argv[0]);
            return 0;
        }
        else if (arg == "--stdin")
        {
            useStdin = true;
        }
        else if (arg == "--verbose" || arg == "-v")
        {
            verbose = true;
        }
        else if (arg == "--json")
        {
            printJson = true;
        }
        else if (arg == "--ast")
        {
            printAst = true;
        }
        else if (arg == "--ir" || arg == "--3ac")
        {
            printIR = true;
        }
        else if (arg == "--quads")
        {
            printQuads = true;
        }
        else if (arg == "--opt")
        {
            printOpt = true;
        }
        else if (arg == "--target" || arg == "--asm" || arg == "--codegen")
        {
            printTarget = true;
        }
        else if (arg == "--all")
        {
            printAll = true;
        }
        else if (arg == "--output" && i + 1 < argc)
        {
            outputFile = argv[++i];
        }
        else if (arg[0] != '-')
        {
            inputFile = arg;
        }
        else
        {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    std::string source;

    try
    {
        if (useStdin)
        {
            std::string line;
            while (std::getline(std::cin, line))
            {
                source += line + "\n";
            }
        }
        else if (!inputFile.empty())
        {
            std::ifstream file(inputFile);
            if (!file.is_open())
            {
                std::cerr << "Error: Cannot open file '" << inputFile << "'" << std::endl;
                return 1;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            source = buffer.str();
            file.close();
        }
        else
        {
            std::cerr << "Error: No input provided" << std::endl;
            printUsage(argv[0]);
            return 1;
        }

        // STAGE 1: Lexical Analysis
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

        if (printAll)
        {
            std::cout << "\n=======================================================\n";
            std::cout << "  STAGE 1: LEXICAL ANALYSIS (TOKEN STREAM)             \n";
            std::cout << "=======================================================\n";
            printTokensAsTable(tokens);
            printStatistics(tokens);

            // STAGE 2: Syntax Analysis
            Parser parser(tokens);
            auto ast = parser.parse();
            ASTPrinter printer;
            std::cout << "=======================================================\n";
            std::cout << "  STAGE 2: SYNTAX ANALYSIS (ABSTRACT SYNTAX TREE)      \n";
            std::cout << "=======================================================\n";
            std::cout << printer.print(*ast) << std::endl;

            // STAGE 3: Semantic Analysis
            SemanticAnalyzer analyzer;
            analyzer.analyze(*ast);
            std::cout << "=======================================================\n";
            std::cout << "  STAGE 3: SEMANTIC ANALYSIS & SYMBOL TABLE            \n";
            std::cout << "=======================================================\n";
            if (analyzer.errors().empty())
            {
                std::cout << "Semantic Verification: PASSED (0 errors)\n\n";
                printSymbolTable(analyzer.symbolTable());
            }
            else
            {
                std::cout << "Semantic Verification: FAILED (" << analyzer.errors().size() << " errors)\n";
                for (const auto &err : analyzer.errors())
                {
                    std::cout << "  - " << err << "\n";
                }
            }

            // STAGE 4: Intermediate Code Generation
            CodeGenerator codegen;
            codegen.generate(*ast);
            std::cout << "\n=======================================================\n";
            std::cout << "  STAGE 4: INTERMEDIATE CODE GENERATION (3AC)          \n";
            std::cout << "=======================================================\n";
            std::cout << "--- Linear Three-Address Code ---\n";
            codegen.code().print(std::cout);

            std::cout << "\n--- Quadruple Table ---\n";
            codegen.code().printQuadrupleTable(std::cout);

            // STAGE 5: Code Optimization
            Optimizer optimizer;
            auto optimizedIR = optimizer.optimize(codegen.code());
            std::cout << "\n=======================================================\n";
            std::cout << "  STAGE 5: CODE OPTIMIZATION (IR TRANSFORMATIONS)      \n";
            std::cout << "=======================================================\n";
            std::cout << "--- Optimized Three-Address Code ---\n";
            optimizedIR.print(std::cout);
            optimizer.printReport(std::cout);

            // STAGE 6: Target Code Generation
            TargetCodeGenerator targetGen;
            targetGen.generate(optimizedIR);
            std::cout << "\n=======================================================\n";
            std::cout << "  STAGE 6: TARGET CODE GENERATION (x86-64 ASSEMBLY)    \n";
            std::cout << "=======================================================\n";
            std::cout << "--- Target Assembly Code Listing ---\n";
            targetGen.printAssembly(std::cout);

            std::cout << "\n--- Stack Frame & Register Storage Mapping Table ---\n";
            targetGen.printAllocationTable(std::cout);
            targetGen.printTargetSummary(std::cout);

            std::cout << "\n=======================================================\n";
            std::cout << "  COMPILER PIPELINE COMPLETE: ALL 6 PHASES PASSED!    \n";
            std::cout << "=======================================================\n";

            if (!outputFile.empty())
            {
                std::ofstream outFile(outputFile);
                if (outFile.is_open())
                {
                    targetGen.printAssembly(outFile);
                    outFile.close();
                    std::cout << "Target Assembly saved to '" << outputFile << "'\n";
                }
            }

            return 0;
        }

        if (printTarget)
        {
            Parser parser(tokens);
            auto ast = parser.parse();
            CodeGenerator codegen;
            codegen.generate(*ast);
            Optimizer optimizer;
            auto optIR = optimizer.optimize(codegen.code());
            TargetCodeGenerator targetGen;
            targetGen.generate(optIR);

            std::cout << "\n=== TARGET x86-64 ASSEMBLY ===" << std::endl;
            targetGen.printAssembly(std::cout);

            std::cout << "\n=== STORAGE & STACK ALLOCATION TABLE ===" << std::endl;
            targetGen.printAllocationTable(std::cout);
            targetGen.printTargetSummary(std::cout);

            if (!outputFile.empty())
            {
                std::ofstream outFile(outputFile);
                if (outFile.is_open())
                {
                    targetGen.printAssembly(outFile);
                    outFile.close();
                    std::cout << "Assembly written to '" << outputFile << "'" << std::endl;
                }
            }
            return 0;
        }

        if (printOpt)
        {
            Parser parser(tokens);
            auto ast = parser.parse();
            CodeGenerator codegen;
            codegen.generate(*ast);
            Optimizer optimizer;
            auto optIR = optimizer.optimize(codegen.code());

            std::cout << "\n=== OPTIMIZED THREE-ADDRESS CODE ===" << std::endl;
            optIR.print(std::cout);
            optimizer.printReport(std::cout);
            return 0;
        }

        if (printIR || printQuads)
        {
            Parser parser(tokens);
            auto ast = parser.parse();
            CodeGenerator codegen;
            codegen.generate(*ast);

            if (printIR)
            {
                std::cout << "\n=== THREE-ADDRESS CODE (3AC) ===" << std::endl;
                codegen.code().print(std::cout);
            }

            if (printQuads)
            {
                std::cout << "\n=== QUADRUPLE TABLE ===" << std::endl;
                codegen.code().printQuadrupleTable(std::cout);
            }

            if (!outputFile.empty())
            {
                std::ofstream outFile(outputFile);
                if (outFile.is_open())
                {
                    codegen.code().print(outFile);
                    if (printQuads)
                    {
                        codegen.code().printQuadrupleTable(outFile);
                    }
                    outFile.close();
                    std::cout << "3AC written to '" << outputFile << "'" << std::endl;
                }
            }
            return 0;
        }

        if (printAst)
        {
            Parser parser(tokens);
            auto ast = parser.parse();
            ASTPrinter printer;
            std::cout << "\n=== ABSTRACT SYNTAX TREE ===" << std::endl;
            std::cout << printer.print(*ast) << std::endl;

            if (!outputFile.empty())
            {
                std::ofstream outFile(outputFile);
                if (outFile.is_open())
                {
                    outFile << printer.print(*ast);
                    outFile.close();
                    std::cout << "AST written to '" << outputFile << "'" << std::endl;
                }
            }
            return 0;
        }

        // Output results
        if (printJson)
        {
            printTokensAsJson(tokens);
        }
        else
        {
            printTokensAsTable(tokens);
        }

        if (verbose)
        {
            printStatistics(tokens);
        }

        if (!outputFile.empty())
        {
            std::ofstream outFile(outputFile);
            if (!outFile.is_open())
            {
                std::cerr << "Error: Cannot open output file '" << outputFile << "'" << std::endl;
                return 1;
            }

            printTokensAsJson(tokens);

            outFile.close();
            std::cout << "Tokens written to '" << outputFile << "'" << std::endl;
        }

        return 0;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Lexical/Syntax Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}