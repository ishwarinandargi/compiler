#pragma once

#include "ast.h"
#include <vector>
#include <memory>
#include <stdexcept>
#include <string>

class ParserError : public std::runtime_error {
public:
    Token token;
    ParserError(const Token& tok, const std::string& message) 
        : std::runtime_error(message), token(tok) {}
};

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);
    
    /**
     * Parse the tokens into an Abstract Syntax Tree.
     * @return The root node of the parsed program.
     */
    std::unique_ptr<ProgramAST> parse();

private:
    std::vector<Token> tokens;
    int current = 0;

    // --- Helper Methods ---
    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    bool check(TokenType type) const;
    Token advance();
    bool match(const std::vector<TokenType>& types);
    Token consume(TokenType type, const std::string& message);
    ParserError error(const Token& token, const std::string& message);
    void synchronize();

    // --- Parsing Rules ---
    
    // Declarations
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<FunctionDecl> functionDeclaration();
    std::unique_ptr<EventDecl> eventDeclaration();
    std::unique_ptr<StateDecl> stateDeclaration();
    std::unique_ptr<ActionDecl> actionDeclaration();
    std::unique_ptr<PropDecl> propDeclaration();
    std::unique_ptr<InitDecl> initDeclaration();
    std::unique_ptr<VarDeclStmt> varDeclaration();

    // Statements
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<BlockStmt> block();
    std::unique_ptr<BranchStmt> branchStatement();
    std::unique_ptr<LoopStmt> loopStatement();
    std::unique_ptr<ReturnStmt> returnStatement();
    std::unique_ptr<Stmt> expressionStatement();

    // Expressions
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logicalOr();
    std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> primary();
};
