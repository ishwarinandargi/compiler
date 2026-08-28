#include "token.h"
#include <cctype>
#include <stdexcept>
#include <sstream>
#include <iomanip>

// ============================================================================
// Token Implementation
// ============================================================================

std::string Token::typeName() const
{
    return tokenTypeToString(type);
}

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
    // Keywords
    case TokenType::CHAR:
        return "CHAR";
    case TokenType::STATE:
        return "STATE";
    case TokenType::EVENT:
        return "EVENT";
    case TokenType::ACTION:
        return "ACTION";
    case TokenType::BRANCH:
        return "BRANCH";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::LOOP:
        return "LOOP";
    case TokenType::FUNC:
        return "FUNC";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::INIT:
        return "INIT";
    case TokenType::PROP:
        return "PROP";

    // Literals
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::STRING:
        return "STRING";

    // Operators
    case TokenType::ASSIGN:
        return "ASSIGN (:=)";
    case TokenType::EQ:
        return "EQ (==)";
    case TokenType::NE:
        return "NE (!=)";
    case TokenType::LT:
        return "LT (<)";
    case TokenType::LE:
        return "LE (<=)";
    case TokenType::GT:
        return "GT (>)";
    case TokenType::GE:
        return "GE (>=)";
    case TokenType::ARROW:
        return "ARROW (->)";
    case TokenType::PLUS:
        return "PLUS (+)";
    case TokenType::MINUS:
        return "MINUS (-)";
    case TokenType::STAR:
        return "STAR (*)";
    case TokenType::SLASH:
        return "SLASH (/)";
    case TokenType::PERCENT:
        return "PERCENT (%)";
    case TokenType::AND:
        return "AND (&&)";
    case TokenType::OR:
        return "OR (||)";
    case TokenType::NOT:
        return "NOT (!)";

    // Delimiters
    case TokenType::LPAREN:
        return "LPAREN (";
    case TokenType::RPAREN:
        return "RPAREN )";
    case TokenType::LBRACE:
        return "LBRACE {";
    case TokenType::RBRACE:
        return "RBRACE }";
    case TokenType::LBRACKET:
        return "LBRACKET [";
    case TokenType::RBRACKET:
        return "RBRACKET ]";
    case TokenType::COLON:
        return "COLON (:)";
    case TokenType::SEMICOLON:
        return "SEMICOLON (;)";
    case TokenType::COMMA:
        return "COMMA (,)";
    case TokenType::DOT:
        return "DOT (.)";
    case TokenType::PIPE:
        return "PIPE (|)";

    // Special
    case TokenType::EOF_TOKEN:
        return "EOF";
    case TokenType::NEWLINE:
        return "NEWLINE";
    case TokenType::UNKNOWN:
        return "UNKNOWN";

    default:
        return "UNKNOWN_TYPE";
    }
}

void printToken(const Token &token)
{
    std::cout << std::setw(20) << std::left << token.typeName()
              << " | '" << token.lexeme << "'"
              << " | L" << token.line << ":C" << token.column << std::endl;
}

void printTokens(const std::vector<Token> &tokens)
{
    std::cout << "\n=== TOKEN STREAM ===" << std::endl;
    std::cout << std::setw(20) << "Token Type"
              << " | Lexeme"
              << " | Location" << std::endl;
    std::cout << std::string(60, '-') << std::endl;

    for (const auto &token : tokens)
    {
        printToken(token);
    }

    std::cout << std::string(60, '-') << std::endl;
    std::cout << "Total tokens: " << tokens.size() << std::endl
              << std::endl;
}

// ============================================================================
// Lexer Implementation
// ============================================================================

Lexer::Lexer(const std::string &source)
    : source(source), pos(0), line(1), column(1), lineStartPos(0) {}

char Lexer::current() const
{
    if (pos >= source.size())
    {
        return '\0';
    }
    return source[pos];
}

char Lexer::peek(int offset) const
{
    if (pos + offset >= source.size())
    {
        return '\0';
    }
    return source[pos + offset];
}

void Lexer::advance()
{
    if (pos < source.size())
    {
        if (source[pos] == '\n')
        {
            line++;
            column = 1;
            lineStartPos = pos + 1;
        }
        else
        {
            column++;
        }
        pos++;
    }
}

void Lexer::skipWhitespace()
{
    while (!isAtEnd() && (current() == ' ' || current() == '\t' || current() == '\r'))
    {
        advance();
    }
}

void Lexer::skipComment()
{
    if (current() == '/' && peek() == '/')
    {
        // Single-line comment: skip to end of line
        while (!isAtEnd() && current() != '\n')
        {
            advance();
        }
    }
}

void Lexer::skipMultilineComment()
{
    if (current() == '/' && peek() == '*')
    {
        advance(); // skip '/'
        advance(); // skip '*'

        while (!isAtEnd())
        {
            if (current() == '*' && peek() == '/')
            {
                advance(); // skip '*'
                advance(); // skip '/'
                return;
            }
            advance();
        }

        throw std::runtime_error("Unterminated multi-line comment starting at line " + std::to_string(line));
    }
}

Token Lexer::readIdentifierOrKeyword()
{
    int startPos = pos;
    int startLine = line;
    int startCol = column;

    // Read identifier: [a-zA-Z_][a-zA-Z0-9_]*
    while (!isAtEnd() && (std::isalnum(current()) || current() == '_'))
    {
        advance();
    }

    std::string lexeme = source.substr(startPos, pos - startPos);

    // Check if it's a keyword
    auto it = KEYWORDS.find(lexeme);
    TokenType type = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;

    return Token(type, lexeme, startLine, startCol, startPos);
}

Token Lexer::readNumber()
{
    int startPos = pos;
    int startLine = line;
    int startCol = column;

    // Read integer part: [0-9]+
    while (!isAtEnd() && std::isdigit(current()))
    {
        advance();
    }

    // Check for decimal point and fractional part
    if (current() == '.' && std::isdigit(peek()))
    {
        advance(); // skip '.'
        while (!isAtEnd() && std::isdigit(current()))
        {
            advance();
        }
    }

    // Check for scientific notation: e/E followed by +/- and digits
    if ((current() == 'e' || current() == 'E') &&
        (std::isdigit(peek()) ||
         ((peek() == '+' || peek() == '-') && std::isdigit(peek(2)))))
    {
        advance(); // skip 'e'/'E'
        if (current() == '+' || current() == '-')
        {
            advance();
        }
        while (!isAtEnd() && std::isdigit(current()))
        {
            advance();
        }
    }

    std::string lexeme = source.substr(startPos, pos - startPos);
    return Token(TokenType::NUMBER, lexeme, startLine, startCol, startPos, lexeme);
}

Token Lexer::readString()
{
    int startPos = pos;
    int startLine = line;
    int startCol = column;

    advance(); // skip opening quote (")

    std::string value;

    while (!isAtEnd() && current() != '"')
    {
        if (current() == '\\')
        {
            // Handle escape sequences
            advance();
            if (isAtEnd())
            {
                throw std::runtime_error("Unterminated string at line " + std::to_string(startLine));
            }

            switch (current())
            {
            case 'n':
                value += '\n';
                break;
            case 't':
                value += '\t';
                break;
            case 'r':
                value += '\r';
                break;
            case '\\':
                value += '\\';
                break;
            case '"':
                value += '"';
                break;
            case '\'':
                value += '\'';
                break;
            case '0':
                value += '\0';
                break;
            default:
                value += current(); // Unknown escape, just include char
            }
            advance();
        }
        else
        {
            value += current();
            advance();
        }
    }

    if (current() != '"')
    {
        throw std::runtime_error("Unterminated string starting at line " + std::to_string(startLine) + ", column " + std::to_string(startCol));
    }

    advance(); // skip closing quote
    std::string lexeme = source.substr(startPos, pos - startPos);
    return Token(TokenType::STRING, lexeme, startLine, startCol, startPos, value);
}

Token Lexer::readCharLiteral()
{
    int startPos = pos;
    int startLine = line;
    int startCol = column;

    advance(); // skip opening quote (')

    std::string value;

    if (isAtEnd() || current() == '\'')
    {
        throw std::runtime_error("Empty character literal at line " + std::to_string(line));
    }

    if (current() == '\\')
    {
        // Handle escape sequences in char literals
        advance();
        if (isAtEnd())
        {
            throw std::runtime_error("Unterminated character literal at line " + std::to_string(startLine));
        }

        switch (current())
        {
        case 'n':
            value += '\n';
            break;
        case 't':
            value += '\t';
            break;
        case 'r':
            value += '\r';
            break;
        case '\\':
            value += '\\';
            break;
        case '\'':
            value += '\'';
            break;
        default:
            value += current();
        }
        advance();
    }
    else
    {
        value += current();
        advance();
    }

    if (current() != '\'')
    {
        throw std::runtime_error("Unterminated character literal at line " + std::to_string(startLine));
    }

    advance(); // skip closing quote
    std::string lexeme = source.substr(startPos, pos - startPos);

    // Return as STRING type (single character)
    return Token(TokenType::STRING, lexeme, startLine, startCol, startPos, value);
}

Token Lexer::makeToken(TokenType type, const std::string &lexeme)
{
    return Token(type, lexeme, line, column - lexeme.length(), pos - lexeme.length());
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespace();

        // Handle comments
        if (current() == '/' && peek() == '/')
        {
            skipComment();
            continue;
        }
        if (current() == '/' && peek() == '*')
        {
            skipMultilineComment();
            continue;
        }

        if (isAtEnd())
            break;

        int startLine = line;
        int startCol = column;
        int startPos = pos;

        char ch = current();

        // Single-character tokens
        switch (ch)
        {
        case '(':
        {
            advance();
            tokens.emplace_back(TokenType::LPAREN, "(", startLine, startCol, startPos);
            continue;
        }
        case ')':
        {
            advance();
            tokens.emplace_back(TokenType::RPAREN, ")", startLine, startCol, startPos);
            continue;
        }
        case '{':
        {
            advance();
            tokens.emplace_back(TokenType::LBRACE, "{", startLine, startCol, startPos);
            continue;
        }
        case '}':
        {
            advance();
            tokens.emplace_back(TokenType::RBRACE, "}", startLine, startCol, startPos);
            continue;
        }
        case '[':
        {
            advance();
            tokens.emplace_back(TokenType::LBRACKET, "[", startLine, startCol, startPos);
            continue;
        }
        case ']':
        {
            advance();
            tokens.emplace_back(TokenType::RBRACKET, "]", startLine, startCol, startPos);
            continue;
        }
        case ',':
        {
            advance();
            tokens.emplace_back(TokenType::COMMA, ",", startLine, startCol, startPos);
            continue;
        }
        case ';':
        {
            advance();
            tokens.emplace_back(TokenType::SEMICOLON, ";", startLine, startCol, startPos);
            continue;
        }
        case '|':
        {
            if (peek() == '|')
            {
                advance();
                advance();
                tokens.emplace_back(TokenType::OR, "||", startLine, startCol, startPos);
            }
            else
            {
                advance();
                tokens.emplace_back(TokenType::PIPE, "|", startLine, startCol, startPos);
            }
            continue;
        }

        // Multi-character tokens
        case ':':
        {
            advance();
            if (current() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::ASSIGN, ":=", startLine, startCol, startPos);
            }
            else
            {
                tokens.emplace_back(TokenType::COLON, ":", startLine, startCol, startPos);
            }
            continue;
        }

        case '.':
        {
            advance();
            if (std::isdigit(current()))
            {
                // Back up and read as number
                pos = startPos;
                column = startCol;
                tokens.push_back(readNumber());
            }
            else
            {
                tokens.emplace_back(TokenType::DOT, ".", startLine, startCol, startPos);
            }
            continue;
        }

        case '=':
        {
            advance();
            if (current() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::EQ, "==", startLine, startCol, startPos);
            }
            else if (current() == '>')
            {
                advance();
                tokens.emplace_back(TokenType::ARROW, "=>", startLine, startCol, startPos);
            }
            else
            {
                throw std::runtime_error("Unexpected character '=' at line " +
                                         std::to_string(startLine) + ", column " +
                                         std::to_string(startCol) + ". Did you mean ':='?");
            }
            continue;
        }

        case '-':
        {
            advance();
            if (current() == '>')
            {
                advance();
                tokens.emplace_back(TokenType::ARROW, "->", startLine, startCol, startPos);
            }
            else if (current() == '-')
            {
                // Could be decrement or second minus
                tokens.emplace_back(TokenType::MINUS, "-", startLine, startCol, startPos);
            }
            else
            {
                tokens.emplace_back(TokenType::MINUS, "-", startLine, startCol, startPos);
            }
            continue;
        }

        case '+':
        {
            advance();
            tokens.emplace_back(TokenType::PLUS, "+", startLine, startCol, startPos);
            continue;
        }

        case '*':
        {
            advance();
            tokens.emplace_back(TokenType::STAR, "*", startLine, startCol, startPos);
            continue;
        }

        case '%':
        {
            advance();
            tokens.emplace_back(TokenType::PERCENT, "%", startLine, startCol, startPos);
            continue;
        }

        case '!':
        {
            advance();
            if (current() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::NE, "!=", startLine, startCol, startPos);
            }
            else
            {
                tokens.emplace_back(TokenType::NOT, "!", startLine, startCol, startPos);
            }
            continue;
        }

        case '<':
        {
            advance();
            if (current() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::LE, "<=", startLine, startCol, startPos);
            }
            else
            {
                tokens.emplace_back(TokenType::LT, "<", startLine, startCol, startPos);
            }
            continue;
        }

        case '>':
        {
            advance();
            if (current() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::GE, ">=", startLine, startCol, startPos);
            }
            else
            {
                tokens.emplace_back(TokenType::GT, ">", startLine, startCol, startPos);
            }
            continue;
        }

        case '&':
        {
            advance();
            if (current() == '&')
            {
                advance();
                tokens.emplace_back(TokenType::AND, "&&", startLine, startCol, startPos);
            }
            else
            {
                throw std::runtime_error("Unexpected character '&' at line " +
                                         std::to_string(startLine) +
                                         ". Did you mean '&&'?");
            }
            continue;
        }

        case '"':
        {
            tokens.push_back(readString());
            continue;
        }

        case '\'':
        {
            tokens.push_back(readCharLiteral());
            continue;
        }

        case '\n':
        {
            // Newlines are tracked for debugging but not typically needed
            tokens.emplace_back(TokenType::NEWLINE, "\\n", startLine, startCol, startPos);
            advance();
            continue;
        }

        default:
            break; // Fall through to identifier/number handling
        }

        // Identifier or keyword
        if (std::isalpha(ch) || ch == '_')
        {
            tokens.push_back(readIdentifierOrKeyword());
        }
        // Number (including decimal starting with digit)
        else if (std::isdigit(ch))
        {
            tokens.push_back(readNumber());
        }
        // Unknown character
        else
        {
            std::stringstream ss;
            ss << "Unexpected character '" << ch << "' (ASCII "
               << static_cast<int>(static_cast<unsigned char>(ch))
               << ") at line " << line << ", column " << column;
            throw std::runtime_error(ss.str());
        }
    }

    // Add EOF token
    tokens.emplace_back(TokenType::EOF_TOKEN, "", line, column, pos);

    return tokens;
}

Token Lexer::peekToken() const
{
    // This would require stateful peeking (more complex)
    // For now, use tokenize() and manage externally
    throw std::runtime_error("peekToken not implemented - use tokenize() instead");
}

Token Lexer::nextToken()
{
    throw std::runtime_error("nextToken not implemented - use tokenize() instead");
}

bool Lexer::isAtEnd() const
{
    return pos >= source.size();
}

void Lexer::reset()
{
    pos = 0;
    line = 1;
    column = 1;
    lineStartPos = 0;
}