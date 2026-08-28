#include "token.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>

/**
 * StoryLang Lexer CLI
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
    std::cout << "  --output <file>      Write tokens to output file (JSON)" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
    std::cout << "\nExample:" << std::endl;
    std::cout << "  " << programName << " story.slt" << std::endl;
    std::cout << "  " << programName << " --stdin --verbose" << std::endl;
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

int main(int argc, char *argv[])
{
    std::string inputFile;
    std::string outputFile;
    bool useStdin = false;
    bool verbose = false;
    bool printJson = false;

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
            // Read from standard input
            std::string line;
            while (std::getline(std::cin, line))
            {
                source += line + "\n";
            }
        }
        else if (!inputFile.empty())
        {
            // Read from file
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

        // Tokenize
        Lexer lexer(source);
        auto tokens = lexer.tokenize();

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

        // Write to file if requested
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
        std::cerr << "Lexical Error: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}