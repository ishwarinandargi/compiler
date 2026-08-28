#pragma once

#include <string>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <vector>

/**
 * TokenType enumeration for StoryLang lexical analysis
 * Covers keywords, identifiers, literals, operators, and delimiters
 */
enum class TokenType
{
    // Keywords
    CHAR,   // Character declaration
    STATE,  // State variable declaration
    EVENT,  // Event declaration
    ACTION, // Action/statement
    BRANCH, // Conditional branching
    ELSE,   // Else clause
    LOOP,   // Loop construct
    FUNC,   // Function declaration
    RETURN, // Return statement
    INIT,   // Initialization
    PROP,   // Property declaration

    // Literals & Identifiers
    IDENTIFIER, // Variable/function names
    NUMBER,     // Integer or floating-point
    STRING,     // String literal

    // Operators
    ASSIGN,  // :=
    EQ,      // ==
    NE,      // !=
    LT,      // <
    LE,      // <=
    GT,      // >
    GE,      // >=
    ARROW,   // ->
    PLUS,    // +
    MINUS,   // -
    STAR,    // *
    SLASH,   // /
    PERCENT, // %
    AND,     // &&
    OR,      // ||
    NOT,     // !

    // Delimiters
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    LBRACKET,  // [
    RBRACKET,  // ]
    COLON,     // :
    SEMICOLON, // ;
    COMMA,     // ,
    DOT,       // .
    PIPE,      // |

    // Special
    EOF_TOKEN, // End of file
    NEWLINE,   // Line ending (tracked but often skipped)
    UNKNOWN    // Unknown token (error)
};

/**
 * Token structure representing a single lexical unit
 */
struct Token
{
    TokenType type;     // Type of token
    std::string lexeme; // The actual text (e.g., "alice", "CHAR", "123")
    int line;           // Line number in source (1-indexed)
    int column;         // Column number in source (1-indexed)
    int position;       // Absolute offset in source
    std::string value;  // Literal value (for numbers/strings)

    /**
     * Constructor for creating a token
     */
    Token(TokenType t, const std::string &lex, int l, int c, int p,
          const std::string &val = "")
        : type(t), lexeme(lex), line(l), column(c), position(p), value(val) {}

    /**
     * Default constructor
     */
    Token() : type(TokenType::UNKNOWN), line(0), column(0), position(0) {}

    /**
     * String representation for debugging
     */
    std::string toString() const
    {
        std::stringstream ss;
        ss << "Token(" << static_cast<int>(type) << ", '" << lexeme
           << "', L" << line << ":C" << column << ")";
        return ss.str();
    }

    /**
     * Get human-readable type name
     */
    std::string typeName() const;
};

/**
 * Keyword lookup map: string -> TokenType
 * Initialized with all StoryLang keywords
 */
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"CHAR", TokenType::CHAR},
    {"STATE", TokenType::STATE},
    {"EVENT", TokenType::EVENT},
    {"ACTION", TokenType::ACTION},
    {"BRANCH", TokenType::BRANCH},
    {"ELSE", TokenType::ELSE},
    {"LOOP", TokenType::LOOP},
    {"FUNC", TokenType::FUNC},
    {"RETURN", TokenType::RETURN},
    {"INIT", TokenType::INIT},
    {"PROP", TokenType::PROP},
};

/**
 * Lexer class for StoryLang
 * Converts source text into a stream of tokens
 */
class Lexer
{
public:
    /**
     * Constructor: initialize lexer with source code
     * @param source: The StoryLang source code as a string view
     */
    explicit Lexer(const std::string &source);

    /**
     * Tokenize the entire source and return token vector
     * @return Vector of tokens
     * @throw std::runtime_error on lexical errors
     */
    std::vector<Token> tokenize();

    /**
     * Get next token without consuming it
     * @return Next token to be read
     */
    Token peekToken() const;

    /**
     * Consume and return the next token
     * @return The next token
     */
    Token nextToken();

    /**
     * Check if at end of input
     * @return true if position >= source length
     */
    bool isAtEnd() const;

    /**
     * Get current line number
     */
    int getCurrentLine() const { return line; }

    /**
     * Get current column number
     */
    int getCurrentColumn() const { return column; }

    /**
     * Get current position in source
     */
    size_t getCurrentPosition() const { return pos; }

    /**
     * Reset lexer to beginning
     */
    void reset();

private:
    std::string source; // The source code being lexed
    size_t pos;         // Current position in source
    int line;           // Current line (1-indexed)
    int column;         // Current column (1-indexed)
    int lineStartPos;   // Position where current line started

    /**
     * Get character at current position
     * @return Character at pos, or '\0' if at end
     */
    char current() const;

    /**
     * Look ahead at character offset positions ahead
     * @param offset: How many positions to look ahead (default 1)
     * @return Character at pos + offset, or '\0' if beyond end
     */
    char peek(int offset = 1) const;

    /**
     * Move to next character, updating line/column tracking
     */
    void advance();

    /**
     * Skip whitespace (spaces, tabs)
     * Does NOT skip newlines (they're significant in StoryLang)
     */
    void skipWhitespace();

    /**
     * Skip single-line comment (// to end of line)
     */
    void skipComment();

    /**
     * Skip multi-line comment
     * Example: slash-star ... star-slash
     */
    void skipMultilineComment();

    /**
     * Read and return an identifier or keyword token
     * Identifiers: [a-zA-Z_][a-zA-Z0-9_]*
     * Keywords: CHAR, ACTION, BRANCH, etc.
     * @return Token of type IDENTIFIER or keyword type
     */
    Token readIdentifierOrKeyword();

    /**
     * Read and return a number token
     * Supports integers and floating-point: 123, 45.67, .5
     * @return Token of type NUMBER
     * @throw std::runtime_error on invalid format
     */
    Token readNumber();

    /**
     * Read and return a string literal token
     * Handles escape sequences: \n, \t, \", \\
     * @return Token of type STRING
     * @throw std::runtime_error on unterminated string
     */
    Token readString();

    /**
     * Read and return a character literal token
     * Single quotes: 'a'
     * @return Token of type STRING (treated as single-char string)
     * @throw std::runtime_error on invalid char literal
     */
    Token readCharLiteral();

    /**
     * Create a token at current position
     * Helper for single/double-char tokens
     */
    Token makeToken(TokenType type, const std::string &lexeme);
};

/**
 * Helper function to convert TokenType to string name
 * @param type: The token type
 * @return Human-readable name (e.g., "CHAR", "IDENTIFIER", "STRING")
 */
std::string tokenTypeToString(TokenType type);

/**
 * Helper function to print token for debugging
 * @param token: Token to print
 */
void printToken(const Token &token);

/**
 * Helper function to print all tokens (for debugging)
 * @param tokens: Vector of tokens
 */
void printTokens(const std::vector<Token> &tokens);