#pragma once

#include "ast.h"
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);

    std::unique_ptr<Program> parse();

private:
    std::vector<Token> tokens_;
    std::size_t pos_ = 0;

    const Token &current() const;
    const Token &peek(std::size_t offset = 1) const;
    const Token &previous() const;

    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    const Token &advance();
    const Token &consume(TokenType type, const std::string &expectedDescription);

    void skipNewlines();
    std::runtime_error makeError(const std::string &expectedDescription) const;
    std::runtime_error makeError(const Token &token, const std::string &expectedDescription) const;

    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<Declaration> parseDeclaration();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseActionStatement();
    std::unique_ptr<Statement> parseBranchStatement();
    std::unique_ptr<Statement> parseLoopStatement();
    std::unique_ptr<Statement> parseAssignmentStatement();
    std::unique_ptr<Statement> parseReturnStatement();
    std::vector<std::unique_ptr<Statement>> parseBlock();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseFunctionCall(const std::string &callee);
    std::vector<std::unique_ptr<Expr>> parseArguments();
    std::string parseQualifiedName();
    std::string parseTypeName();
    std::vector<std::string> parseParameterList();
>>>>>>> b150139 (Added Parser Implementation)
};
