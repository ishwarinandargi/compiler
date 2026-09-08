#include "parser.h"
#include <iostream>

Parser::Parser(const std::vector<Token>& tokens) : tokens(tokens) {}

std::unique_ptr<ProgramAST> Parser::parse() {
    std::vector<std::unique_ptr<Stmt>> declarations;
    while (!isAtEnd() && !check(TokenType::EOF_TOKEN)) {
        if (match({TokenType::NEWLINE})) continue;
        
        try {
            declarations.push_back(declaration());
        } catch (const ParserError& error) {
            std::cerr << "Syntax Error at line " << error.token.line << ": " << error.what() << std::endl;
            synchronize();
        }
    }
    return std::make_unique<ProgramAST>(std::move(declarations));
}

// --- Helpers ---

bool Parser::isAtEnd() const {
    return current >= tokens.size() || tokens[current].type == TokenType::EOF_TOKEN;
}

Token Parser::peek() const {
    if (isAtEnd()) return tokens.back(); // Return EOF token
    return tokens[current];
}

Token Parser::previous() const {
    return tokens[current - 1];
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::match(const std::vector<TokenType>& types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

ParserError Parser::error(const Token& token, const std::string& message) {
    return ParserError(token, message);
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::SEMICOLON) return;
        
        switch (peek().type) {
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
        advance();
    }
}

// --- Declarations ---

std::unique_ptr<Stmt> Parser::declaration() {
    if (match({TokenType::FUNC})) return functionDeclaration();
    if (match({TokenType::EVENT})) return eventDeclaration();
    if (match({TokenType::STATE})) return stateDeclaration();
    if (match({TokenType::ACTION})) return actionDeclaration();
    if (match({TokenType::PROP})) return propDeclaration();
    if (match({TokenType::INIT})) return initDeclaration();
    if (match({TokenType::CHAR})) return varDeclaration();

    return statement();
}

std::unique_ptr<FunctionDecl> Parser::functionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect function name.");
    consume(TokenType::LPAREN, "Expect '(' after function name.");
    
    std::vector<Token> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Can't have more than 255 parameters.");
            }
            parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
        } while (match({TokenType::COMMA}));
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<FunctionDecl>(name, parameters, std::move(body));
}

std::unique_ptr<EventDecl> Parser::eventDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect event name.");
    
    std::vector<Token> parameters;
    if (match({TokenType::LPAREN})) {
        if (!check(TokenType::RPAREN)) {
            do {
                parameters.push_back(consume(TokenType::IDENTIFIER, "Expect parameter name."));
            } while (match({TokenType::COMMA}));
        }
        consume(TokenType::RPAREN, "Expect ')' after parameters.");
    }
    
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<EventDecl>(name, parameters, std::move(body));
}

std::unique_ptr<StateDecl> Parser::stateDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect state name.");
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::ASSIGN, TokenType::EQ})) { // Accept := or = for state initialization
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after state declaration.");
    return std::make_unique<StateDecl>(name, std::move(initializer));
}

std::unique_ptr<ActionDecl> Parser::actionDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect action name.");
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<ActionDecl>(name, std::move(body));
}

std::unique_ptr<PropDecl> Parser::propDeclaration() {
    Token name = consume(TokenType::IDENTIFIER, "Expect property name.");
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::ASSIGN, TokenType::EQ})) {
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after property declaration.");
    return std::make_unique<PropDecl>(name, std::move(initializer));
}

std::unique_ptr<InitDecl> Parser::initDeclaration() {
    std::unique_ptr<BlockStmt> body = block();
    return std::make_unique<InitDecl>(std::move(body));
}

std::unique_ptr<VarDeclStmt> Parser::varDeclaration() {
    Token typeKw = previous(); // CHAR
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");
    
    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenType::ASSIGN, TokenType::EQ})) {
        initializer = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    return std::make_unique<VarDeclStmt>(typeKw, name, std::move(initializer));
}

// --- Statements ---

std::unique_ptr<Stmt> Parser::statement() {
    if (match({TokenType::BRANCH})) return branchStatement();
    if (match({TokenType::LOOP})) return loopStatement();
    if (match({TokenType::RETURN})) return returnStatement();
    if (match({TokenType::LBRACE})) return block();

    return expressionStatement();
}

std::unique_ptr<BlockStmt> Parser::block() {
    if (previous().type != TokenType::LBRACE) {
        consume(TokenType::LBRACE, "Expect '{' before block.");
    }
    
    std::vector<std::unique_ptr<Stmt>> statements;
    
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        if (match({TokenType::NEWLINE})) continue;
        statements.push_back(declaration());
    }
    
    consume(TokenType::RBRACE, "Expect '}' after block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<BranchStmt> Parser::branchStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'BRANCH'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after condition.");
    
    std::unique_ptr<Stmt> thenBranch = statement();
    std::unique_ptr<Stmt> elseBranch = nullptr;
    
    if (match({TokenType::ELSE})) {
        elseBranch = statement();
    }
    
    return std::make_unique<BranchStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<LoopStmt> Parser::loopStatement() {
    consume(TokenType::LPAREN, "Expect '(' after 'LOOP'.");
    std::unique_ptr<Expr> condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after condition.");
    
    std::unique_ptr<Stmt> body = statement();
    return std::make_unique<LoopStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<ReturnStmt> Parser::returnStatement() {
    Token keyword = previous();
    std::unique_ptr<Expr> value = nullptr;
    
    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }
    
    consume(TokenType::SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(keyword, std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
    std::unique_ptr<Expr> expr = expression();
    
    // In StoryLang, := is the assignment operator. 
    // We treat it as an assignment expression, but if it sits alone, it's an expr statement.
    if (match({TokenType::ASSIGN})) { // := 
        if (auto* idExpr = dynamic_cast<IdentifierExpr*>(expr.get())) {
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

std::unique_ptr<Expr> Parser::expression() {
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment() {
    return logicalOr();
    // Assignment handling is done in expressionStatement for AssignStmt,
    // or we could support inline assignment here. Given AssignStmt exists, 
    // we'll handle `a := 5` via AssignStmt.
}

std::unique_ptr<Expr> Parser::logicalOr() {
    std::unique_ptr<Expr> expr = logicalAnd();
    
    while (match({TokenType::OR})) {
        Token op = previous();
        std::unique_ptr<Expr> right = logicalAnd();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
    std::unique_ptr<Expr> expr = equality();
    
    while (match({TokenType::AND})) {
        Token op = previous();
        std::unique_ptr<Expr> right = equality();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::equality() {
    std::unique_ptr<Expr> expr = comparison();
    
    while (match({TokenType::EQ, TokenType::NE})) {
        Token op = previous();
        std::unique_ptr<Expr> right = comparison();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
    std::unique_ptr<Expr> expr = term();
    
    while (match({TokenType::GT, TokenType::GE, TokenType::LT, TokenType::LE})) {
        Token op = previous();
        std::unique_ptr<Expr> right = term();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::term() {
    std::unique_ptr<Expr> expr = factor();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = factor();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::factor() {
    std::unique_ptr<Expr> expr = unary();
    
    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        expr = std::make_unique<BinaryExpr>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS})) {
        Token op = previous();
        std::unique_ptr<Expr> right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    
    return call();
}

std::unique_ptr<Expr> Parser::call() {
    std::unique_ptr<Expr> expr = primary();
    
    while (true) {
        if (match({TokenType::LPAREN})) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(expression());
                } while (match({TokenType::COMMA}));
            }
            Token paren = consume(TokenType::RPAREN, "Expect ')' after arguments.");
            expr = std::make_unique<CallExpr>(std::move(expr), paren, std::move(args));
        } else {
            break;
        }
    }
    
    return expr;
}

std::unique_ptr<Expr> Parser::primary() {
    if (match({TokenType::NUMBER})) {
        return std::make_unique<NumberExpr>(previous(), std::stod(previous().value));
    }
    if (match({TokenType::STRING})) {
        return std::make_unique<StringExpr>(previous(), previous().value);
    }
    if (match({TokenType::IDENTIFIER})) {
        return std::make_unique<IdentifierExpr>(previous());
    }
    if (match({TokenType::LPAREN})) {
        std::unique_ptr<Expr> expr = expression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return expr;
    }
    
    throw error(peek(), "Expect expression.");
}
