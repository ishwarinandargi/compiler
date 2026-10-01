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
>>>>>>> b150139 (Added Parser Implementation)
    }
    return false;
}

<<<<<<< HEAD
Token Parser::consume(TokenType type, const std::string &message)
{
    if (check(type))
        return advance();
    throw error(peek(), message);
}

ParserError Parser::error(const Token &token, const std::string &message)
{
    return ParserError(token, message);
}

void Parser::synchronize()
{
    advance();
    while (!isAtEnd())
    {
        if (previous().type == TokenType::SEMICOLON)
            return;

        switch (peek().type)
        {
        case TokenType::CHAR:
        case TokenType::STATE:
        case TokenType::EVENT:
        case TokenType::ACTION:
        case TokenType::BRANCH:
        case TokenType::LOOP:
        case TokenType::FUNC:
        case TokenType::RETURN:
        case TokenType::INIT:
        case TokenType::PROP:
            return;
        default:
            break;
        }
        == == == =
                     const Token & Parser::advance()
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
>>>>>>> b150139 (Added Parser Implementation)
                advance();
            }
        }

<<<<<<< HEAD
        // --- Declarations ---

        std::unique_ptr<Stmt> Parser::declaration()
        {
            if (match({TokenType::FUNC}))
                return functionDeclaration();
            if (match({TokenType::EVENT}))
                return eventDeclaration();
            if (match({TokenType::STATE}))
                return stateDeclaration();
            if (match({TokenType::ACTION}))
                return actionDeclaration();
            if (match({TokenType::PROP}))
                return propDeclaration();
            if (match({TokenType::INIT}))
                return initDeclaration();
            if (match({TokenType::CHAR}))
                return varDeclaration();

            return statement();
        }

        std::unique_ptr<FunctionDecl> Parser::functionDeclaration()
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
            consume(TokenType::LPAREN, "Expect '(' after function name.");

            std::vector<Token> parameters;
            if (!check(TokenType::RPAREN))
            {
                do
                {
                    if (parameters.size() >= 255)
                    {
                        error(peek(), "Can't have more than 255 parameters.");
                    }
                    parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
                } while (match({TokenType::COMMA}));
            }
            consume(TokenType::RPAREN, "Expect ')' after parameters.");

            std::unique_ptr<BlockStmt> body = block();
            return std::make_unique<FunctionDecl>(name, parameters, std::move(body));
        }

        std::unique_ptr<EventDecl> Parser::eventDeclaration()
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect event name.");

            std::vector<Token> parameters;
            if (match({TokenType::LPAREN}))
            {
                if (!check(TokenType::RPAREN))
                {
                    do
                    {
                        parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
                    } while (match({TokenType::COMMA}));
                }
                consume(TokenType::RPAREN, "Expect ')' after parameters.");
            }

            std::unique_ptr<BlockStmt> body = block();
            return std::make_unique<EventDecl>(name, parameters, std::move(body));
        }

        std::unique_ptr<StateDecl> Parser::stateDeclaration()
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect state name.");

            std::unique_ptr<Expr> initializer = nullptr;
            if (match({TokenType::ASSIGN, TokenType::EQ}))
            { // Accept := or = for state initialization
                initializer = expression();
            }

            consume(TokenType::SEMICOLON, "Expect ';' after state declaration.");
            return std::make_unique<StateDecl>(name, std::move(initializer));
        }

        std::unique_ptr<ActionDecl> Parser::actionDeclaration()
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect action name.");
            std::unique_ptr<BlockStmt> body = block();
            return std::make_unique<ActionDecl>(name, std::move(body));
        }

        std::unique_ptr<PropDecl> Parser::propDeclaration()
        {
            Token name = consume(TokenType::IDENTIFIER, "Expect property name.");

            std::unique_ptr<Expr> initializer = nullptr;
            if (match({TokenType::ASSIGN, TokenType::EQ}))
            {
                initializer = expression();
            }

            consume(TokenType::SEMICOLON, "Expect ';' after property declaration.");
            return std::make_unique<PropDecl>(name, std::move(initializer));
        }

        std::unique_ptr<InitDecl> Parser::initDeclaration()
        {
            std::unique_ptr<BlockStmt> body = block();
            return std::make_unique<InitDecl>(std::move(body));
        }

        std::unique_ptr<VarDeclStmt> Parser::varDeclaration()
        {
            Token typeKw = previous(); // CHAR
            Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

            std::unique_ptr<Expr> initializer = nullptr;
            if (match({TokenType::ASSIGN, TokenType::EQ}))
            {
                initializer = expression();
            }

            consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
            return std::make_unique<VarDeclStmt>(typeKw, name, std::move(initializer));
        }

        // --- Statements ---

        std::unique_ptr<Stmt> Parser::statement()
        {
            if (match({TokenType::BRANCH}))
                return branchStatement();
            if (match({TokenType::LOOP}))
                return loopStatement();
            if (match({TokenType::RETURN}))
                return returnStatement();
            if (match({TokenType::LBRACE}))
                return block();

            return expressionStatement();
        }

        std::unique_ptr<BlockStmt> Parser::block()
        {
            if (previous().type != TokenType::LBRACE)
            {
                consume(TokenType::LBRACE, "Expect '{' before block.");
            }

            std::vector<std::unique_ptr<Stmt>> statements;

            while (!check(TokenType::RBRACE) && !isAtEnd())
            {
                if (match({TokenType::NEWLINE}))
                    continue;
                statements.push_back(declaration());
            }

            consume(TokenType::RBRACE, "Expect '}' after block.");
            return std::make_unique<BlockStmt>(std::move(statements));
        }

        std::unique_ptr<BranchStmt> Parser::branchStatement()
        {
            consume(TokenType::LPAREN, "Expect '(' after 'BRANCH'.");
            std::unique_ptr<Expr> condition = expression();
            consume(TokenType::RPAREN, "Expect ')' after condition.");

            std::unique_ptr<Stmt> thenBranch = statement();
            std::unique_ptr<Stmt> elseBranch = nullptr;

            if (match({TokenType::ELSE}))
            {
                elseBranch = statement();
            }

            return std::make_unique<BranchStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
        }

        std::unique_ptr<LoopStmt> Parser::loopStatement()
        {
            consume(TokenType::LPAREN, "Expect '(' after 'LOOP'.");
            std::unique_ptr<Expr> condition = expression();
            consume(TokenType::RPAREN, "Expect ')' after condition.");

            std::unique_ptr<Stmt> body = statement();
            return std::make_unique<LoopStmt>(std::move(condition), std::move(body));
        }

        std::unique_ptr<ReturnStmt> Parser::returnStatement()
        {
            Token keyword = previous();
            std::unique_ptr<Expr> value = nullptr;

            if (!check(TokenType::SEMICOLON))
            {
                value = expression();
            }

            consume(TokenType::SEMICOLON, "Expect ';' after return value.");
            return std::make_unique<ReturnStmt>(keyword, std::move(value));
        }

        std::unique_ptr<Stmt> Parser::expressionStatement()
        {
            std::unique_ptr<Expr> expr = expression();

            // In StoryLang, := is the assignment operator.
            // We treat it as an assignment expression, but if it sits alone, it's an expr statement.
            if (match({TokenType::ASSIGN}))
            { // :=
                if (auto *idExpr = dynamic_cast<IdentifierExpr *>(expr.get()))
                {
                    Token name = idExpr->name;
                    std::unique_ptr<Expr> value = expression();
                    consume(TokenType::SEMICOLON, "Expect ';' after assignment.");
                    return std::make_unique<AssignStmt>(name, std::move(value));
                }
                error(previous(), "Invalid assignment target.");
            }

            consume(TokenType::SEMICOLON, "Expect ';' after expression.");
            return std::make_unique<ExprStmt>(std::move(expr));
        }

        // --- Expressions ---

        std::unique_ptr<Expr> Parser::expression()
        {
            return assignment();
        }

        std::unique_ptr<Expr> Parser::assignment()
        {
            return logicalOr();
            // Assignment handling is done in expressionStatement for AssignStmt,
            // or we could support inline assignment here. Given AssignStmt exists,
            // we'll handle `a := 5` via AssignStmt.
        }

        std::unique_ptr<Expr> Parser::logicalOr()
        {
            std::unique_ptr<Expr> expr = logicalAnd();

            while (match({TokenType::OR}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = logicalAnd();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::logicalAnd()
        {
            std::unique_ptr<Expr> expr = equality();

            while (match({TokenType::AND}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = equality();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::equality()
        {
            std::unique_ptr<Expr> expr = comparison();

            while (match({TokenType::EQ, TokenType::NE}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = comparison();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::comparison()
        {
            std::unique_ptr<Expr> expr = term();

            while (match({TokenType::GT, TokenType::GE, TokenType::LT, TokenType::LE}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = term();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::term()
        {
            std::unique_ptr<Expr> expr = factor();

            while (match({TokenType::MINUS, TokenType::PLUS}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = factor();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::factor()
        {
            std::unique_ptr<Expr> expr = unary();

            while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = unary();
                expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::unary()
        {
            if (match({TokenType::NOT, TokenType::MINUS}))
            {
                Token op = previous();
                std::unique_ptr<Expr> right = unary();
                return std::make_unique<UnaryExpr>(op, std::move(right));
            }

            return call();
        }

        std::unique_ptr<Expr> Parser::call()
        {
            std::unique_ptr<Expr> expr = primary();

            while (true)
            {
                if (match({TokenType::LPAREN}))
                {
                    std::vector<std::unique_ptr<Expr>> args;
                    if (!check(TokenType::RPAREN))
                    {
                        do
                        {
                            args.push_back(expression());
                        } while (match({TokenType::COMMA}));
                    }
                    Token paren = consume(TokenType::RPAREN, "Expect ')' after arguments.");
                    expr = std::make_unique<CallExpr>(std::move(expr), paren, std::move(args));
                }
                else
                {
                    break;
                }
            }

            return expr;
        }

        std::unique_ptr<Expr> Parser::primary()
        {
            if (match({TokenType::NUMBER}))
            {
                return std::make_unique<NumberExpr>(previous(), std::stod(previous().value));
            }
            if (match({TokenType::STRING}))
            {
                return std::make_unique<StringExpr>(previous(), previous().value);
            }
            if (match({TokenType::IDENTIFIER}))
            {
                return std::make_unique<IdentifierExpr>(previous());
            }
            if (match({TokenType::LPAREN}))
            {
                std::unique_ptr<Expr> expr = expression();
                consume(TokenType::RPAREN, "Expect ')' after expression.");
                return expr;
            }

            throw error(peek(), "Expect expression.");
            == == == =
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
                const std::string name = consume(TokenType::IDENTIFIER, "identifier").lexeme;
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
