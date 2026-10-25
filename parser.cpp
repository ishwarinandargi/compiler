#include "parser.h"
#include <utility>

void IdentifierExpr::accept(Visitor &visitor) { visitor.visit(*this); }
void LiteralExpr::accept(Visitor &visitor) { visitor.visit(*this); }
void BinaryExpr::accept(Visitor &visitor) { visitor.visit(*this); }
void UnaryExpr::accept(Visitor &visitor) { visitor.visit(*this); }
void CallExpr::accept(Visitor &visitor) { visitor.visit(*this); }
void ActionStmt::accept(Visitor &visitor) { visitor.visit(*this); }
void BranchStmt::accept(Visitor &visitor) { visitor.visit(*this); }
void LoopStmt::accept(Visitor &visitor) { visitor.visit(*this); }
void AssignmentStmt::accept(Visitor &visitor) { visitor.visit(*this); }
void ReturnStmt::accept(Visitor &visitor) { visitor.visit(*this); }
void CharDecl::accept(Visitor &visitor) { visitor.visit(*this); }
void StateDecl::accept(Visitor &visitor) { visitor.visit(*this); }
void EventDecl::accept(Visitor &visitor) { visitor.visit(*this); }
void FuncDecl::accept(Visitor &visitor) { visitor.visit(*this); }

Parser::Parser(const std::vector<Token> &tokens) : tokens_(tokens) {}

const Token &Parser::current() const
{
    if (tokens_.empty())
    {
        static Token empty(TokenType::EOF_TOKEN, "", 0, 0, 0);
        return empty;
    }
    if (pos_ >= tokens_.size())
    {
        return tokens_.back();
    }
    return tokens_[pos_];
}

const Token &Parser::peek(std::size_t offset) const
{
    const std::size_t index = pos_ + offset;
    if (index >= tokens_.size())
    {
        return tokens_.back();
    }
    return tokens_[index];
}

const Token &Parser::previous() const
{
    if (pos_ == 0)
    {
        return tokens_.front();
    }
    return tokens_[pos_ - 1];
}

bool Parser::isAtEnd() const
{
    if (tokens_.empty() || pos_ >= tokens_.size())
    {
        return true;
    }
    return tokens_[pos_].type == TokenType::EOF_TOKEN;
}

bool Parser::check(TokenType type) const
{
    return current().type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

const Token &Parser::advance()
{
    if (!isAtEnd())
    {
        ++pos_;
    }
    return previous();
}

const Token &Parser::consume(TokenType type, const std::string &expectedDescription)
{
    if (!check(type))
    {
        throw makeError(expectedDescription);
    }
    return advance();
}

void Parser::skipNewlines()
{
    while (!isAtEnd() && check(TokenType::NEWLINE))
    {
        advance();
    }
}

                         std::runtime_error Parser::makeError(const std::string &expectedDescription) const
            {
                return makeError(current(), expectedDescription);
            }

            std::runtime_error Parser::makeError(const Token &token, const std::string &expectedDescription) const
            {
                std::string actual = token.lexeme.empty() ? token.typeName() : token.lexeme;
                return std::runtime_error("Expected " + expectedDescription + " at line " +
                                          std::to_string(token.line) + ", column " +
                                          std::to_string(token.column) + ". Got '" + actual + "' instead.");
            }

            std::unique_ptr<Program> Parser::parse()
            {
                auto program = parseProgram();
                skipNewlines();
                if (!isAtEnd())
                {
                    throw makeError(current(), "end of program");
                }
                return program;
            }

            std::unique_ptr<Program> Parser::parseProgram()
            {
                auto program = std::make_unique<Program>();

                while (!isAtEnd())
                {
                    skipNewlines();
                    if (isAtEnd())
                    {
                        break;
                    }

                    if (check(TokenType::CHAR) || check(TokenType::STATE) || check(TokenType::EVENT) ||
                        check(TokenType::FUNC) || check(TokenType::PROP))
                    {
                        program->declarations.push_back(parseDeclaration());
                        continue;
                    }

                    if (check(TokenType::ACTION) || check(TokenType::BRANCH) || check(TokenType::LOOP) ||
                        check(TokenType::RETURN) || check(TokenType::INIT) || check(TokenType::IDENTIFIER))
                    {
                        program->statements.push_back(parseStatement());
                        continue;
                    }

                    throw makeError(current(), "statement or declaration");
                }

                return program;
            }

            std::unique_ptr<Declaration> Parser::parseDeclaration()
            {
                if (match(TokenType::CHAR))
                {
                    const auto &nameToken = consume(TokenType::IDENTIFIER, "identifier");
                    consume(TokenType::COLON, ":");
                    const std::string typeName = parseTypeName();
                    return std::make_unique<CharDecl>(nameToken.lexeme, typeName);
                }

                if (match(TokenType::STATE))
                {
                    const std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
                    consume(TokenType::COLON, ":");
                    consume(TokenType::LBRACE, "{");

                    std::vector<std::string> values;
                    if (!check(TokenType::RBRACE))
                    {
                        values.push_back(consume(TokenType::IDENTIFIER, "state value").lexeme);
                        while (match(TokenType::PIPE) || match(TokenType::COMMA))
                        {
                            values.push_back(consume(TokenType::IDENTIFIER, "state value").lexeme);
                        }
                    }
                    consume(TokenType::RBRACE, "}");
                    return std::make_unique<StateDecl>(name, values);
                }

                if (match(TokenType::EVENT))
                {
                    const std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
                    consume(TokenType::COLON, ":");
                    auto body = parseBlock();
                    return std::make_unique<EventDecl>(name, std::move(body));
                }

                if (match(TokenType::FUNC))
                {
                    const std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
                    consume(TokenType::LPAREN, "(");
                    std::vector<std::string> params;
                    if (!check(TokenType::RPAREN))
                    {
                        params = parseParameterList();
                    }
                    consume(TokenType::RPAREN, ")");
                    consume(TokenType::COLON, ":");
                    auto body = parseBlock();
                    return std::make_unique<FuncDecl>(name, params, std::move(body));
                }

                if (match(TokenType::PROP))
                {
                    const std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
                    consume(TokenType::COLON, ":");
                    auto value = parseExpression();
                    return std::make_unique<FuncDecl>(name, std::vector<std::string>(), std::vector<std::unique_ptr<Statement>>());
                }

                throw makeError(current(), "declaration");
            }

            std::unique_ptr<Statement> Parser::parseStatement()
            {
                if (match(TokenType::ACTION))
                {
                    return parseActionStatement();
                }

                if (match(TokenType::BRANCH))
                {
                    return parseBranchStatement();
                }

                if (match(TokenType::LOOP))
                {
                    return parseLoopStatement();
                }

                if (match(TokenType::RETURN))
                {
                    return parseReturnStatement();
                }

                if (match(TokenType::INIT))
                {
                    const std::string lvalue = parseQualifiedName();
                    consume(TokenType::ASSIGN, ":=");
                    auto value = parseExpression();
                    return std::make_unique<AssignmentStmt>(lvalue, std::move(value));
                }

                if (check(TokenType::IDENTIFIER))
                {
                    const std::string lvalue = parseQualifiedName();
                    if (match(TokenType::ASSIGN))
                    {
                        auto value = parseExpression();
                        return std::make_unique<AssignmentStmt>(lvalue, std::move(value));
                    }

                    throw makeError(current(), "assignment operator ':=' or statement keyword");
                }

                throw makeError(current(), "statement");
            }

            std::unique_ptr<Statement> Parser::parseActionStatement()
            {
                const std::string name = parseQualifiedName();
                consume(TokenType::LPAREN, "(");
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RPAREN))
                {
                    args = parseArguments();
                }
                consume(TokenType::RPAREN, ")");
                return std::make_unique<ActionStmt>(name, std::move(args));
            }

            std::unique_ptr<Statement> Parser::parseBranchStatement()
            {
                auto condition = parseExpression();
                consume(TokenType::COLON, ":");
                auto thenBlock = parseBlock();

                std::vector<std::unique_ptr<Statement>> elseBlock;
                if (match(TokenType::ELSE))
                {
                    consume(TokenType::COLON, ":");
                    elseBlock = parseBlock();
                }

                return std::make_unique<BranchStmt>(std::move(condition), std::move(thenBlock), std::move(elseBlock));
            }

            std::unique_ptr<Statement> Parser::parseLoopStatement()
            {
                auto condition = parseExpression();
                consume(TokenType::COLON, ":");
                auto body = parseBlock();
                return std::make_unique<LoopStmt>(std::move(condition), std::move(body));
            }

            std::unique_ptr<Statement> Parser::parseAssignmentStatement()
            {
                const std::string lvalue = parseQualifiedName();
                consume(TokenType::ASSIGN, ":=");
                auto value = parseExpression();
                return std::make_unique<AssignmentStmt>(lvalue, std::move(value));
            }

            std::unique_ptr<Statement> Parser::parseReturnStatement()
            {
                auto value = parseExpression();
                return std::make_unique<ReturnStmt>(std::move(value));
            }

            std::vector<std::unique_ptr<Statement>> Parser::parseBlock()
            {
                std::vector<std::unique_ptr<Statement>> statements;

                while (!isAtEnd() && !check(TokenType::ELSE) && !check(TokenType::EOF_TOKEN) && !check(TokenType::RBRACE))
                {
                    skipNewlines();
                    if (isAtEnd() || check(TokenType::ELSE) || check(TokenType::RBRACE))
                    {
                        break;
                    }
                    statements.push_back(parseStatement());
                }

                return statements;
            }

            std::unique_ptr<Expr> Parser::parseExpression()
            {
                return parseLogicalOr();
            }

            std::unique_ptr<Expr> Parser::parseLogicalOr()
            {
                auto expr = parseLogicalAnd();
                while (match(TokenType::OR))
                {
                    const std::string op = previous().lexeme;
                    auto right = parseLogicalAnd();
                    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expr> Parser::parseLogicalAnd()
            {
                auto expr = parseComparison();
                while (match(TokenType::AND))
                {
                    const std::string op = previous().lexeme;
                    auto right = parseComparison();
                    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expr> Parser::parseComparison()
            {
                auto expr = parseAdditive();
                while (check(TokenType::EQ) || check(TokenType::NE) || check(TokenType::LT) ||
                       check(TokenType::LE) || check(TokenType::GT) || check(TokenType::GE))
                {
                    const std::string op = advance().lexeme;
                    auto right = parseAdditive();
                    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expr> Parser::parseAdditive()
            {
                auto expr = parseMultiplicative();
                while (check(TokenType::PLUS) || check(TokenType::MINUS))
                {
                    const std::string op = advance().lexeme;
                    auto right = parseMultiplicative();
                    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expr> Parser::parseMultiplicative()
            {
                auto expr = parseUnary();
                while (check(TokenType::STAR) || check(TokenType::SLASH) || check(TokenType::PERCENT))
                {
                    const std::string op = advance().lexeme;
                    auto right = parseUnary();
                    expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expr> Parser::parseUnary()
            {
                if (match(TokenType::NOT) || match(TokenType::MINUS))
                {
                    const std::string op = previous().lexeme;
                    auto operand = parseUnary();
                    return std::make_unique<UnaryExpr>(op, std::move(operand));
                }
                return parsePrimary();
            }

            std::unique_ptr<Expr> Parser::parsePrimary()
            {
                if (match(TokenType::LPAREN))
                {
                    auto expr = parseExpression();
                    consume(TokenType::RPAREN, ")");
                    return expr;
                }

                if (check(TokenType::NUMBER))
                {
                    const auto &token = advance();
                    return std::make_unique<LiteralExpr>(TokenType::NUMBER, token.lexeme);
                }

                if (check(TokenType::STRING))
                {
                    const auto &token = advance();
                    return std::make_unique<LiteralExpr>(TokenType::STRING, token.value.empty() ? token.lexeme : token.value);
                }

                if (check(TokenType::IDENTIFIER))
                {
                    const std::string callee = parseQualifiedName();
                    if (check(TokenType::LPAREN))
                    {
                        return parseFunctionCall(callee);
                    }
                    return std::make_unique<IdentifierExpr>(callee);
                }

                throw makeError(current(), "expression");
            }

            std::unique_ptr<Expr> Parser::parseFunctionCall(const std::string &callee)
            {
                consume(TokenType::LPAREN, "(");
                std::vector<std::unique_ptr<Expr>> args;
                if (!check(TokenType::RPAREN))
                {
                    args = parseArguments();
                }
                consume(TokenType::RPAREN, ")");
                return std::make_unique<CallExpr>(callee, std::move(args));
            }

            std::vector<std::unique_ptr<Expr>> Parser::parseArguments()
            {
                std::vector<std::unique_ptr<Expr>> args;
                args.push_back(parseExpression());
                while (match(TokenType::COMMA))
                {
                    args.push_back(parseExpression());
                }
                return args;
            }

            std::string Parser::parseQualifiedName()
            {
                std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
                while (match(TokenType::DOT))
                {
                    name += "." + consume(TokenType::IDENTIFIER, "identifier").lexeme;
                }
                return name;
            }

            std::string Parser::parseTypeName()
            {
                if (check(TokenType::IDENTIFIER))
                {
                    return consume(TokenType::IDENTIFIER, "identifier").lexeme;
                }
                if (check(TokenType::STRING))
                {
                    const auto &token = advance();
                    return token.value.empty() ? token.lexeme : token.value;
                }
                throw makeError(current(), "type name");
            }

            std::vector<std::string> Parser::parseParameterList()
            {
                std::vector<std::string> params;
                params.push_back(consume(TokenType::IDENTIFIER, "identifier").lexeme);
                while (match(TokenType::COMMA))
                {
                    params.push_back(consume(TokenType::IDENTIFIER, "identifier").lexeme);
                }
                return params;
            }
