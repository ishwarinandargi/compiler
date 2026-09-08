#include "parser.h"
#include "ast.h"
#include <iostream>
#include <cassert>
#include <string>

// Lexer header wasn't defined, but Lexer class is in token.h (as shown by lexer_test.cpp).
#include "token.h"

// Helper function to parse a string and return the string representation of AST
std::string parseAndPrint(const std::string& source) {
    try {
        Lexer lexer(source);
        auto tokens = lexer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parse();
        
        ASTPrinter printer;
        return printer.print(*ast);
    } catch (const std::exception& e) {
        return std::string("ERROR: ") + e.what();
    }
}

void testVarDecl() {
    std::string source = "CHAR hero := \"Alice\";\n";
    std::string result = parseAndPrint(source);
    
    assert(result.find("VarDeclStmt (CHAR hero)") != std::string::npos);
    assert(result.find("StringExpr (\"Alice\")") != std::string::npos);
    std::cout << "testVarDecl passed!" << std::endl;
}

void testStateDecl() {
    std::string source = "STATE playing;\nSTATE game_over := 0;\n";
    std::string result = parseAndPrint(source);
    
    assert(result.find("StateDecl (playing)") != std::string::npos);
    assert(result.find("StateDecl (game_over)") != std::string::npos);
    assert(result.find("NumberExpr (0.000000)") != std::string::npos);
    std::cout << "testStateDecl passed!" << std::endl;
}

void testEventDecl() {
    std::string source = "EVENT on_start(hero, villain) { CHAR health := 100; }\n";
    std::string result = parseAndPrint(source);
    
    assert(result.find("EventDecl (on_start) Params: [hero, villain]") != std::string::npos);
    assert(result.find("VarDeclStmt (CHAR health)") != std::string::npos);
    std::cout << "testEventDecl passed!" << std::endl;
}

void testExpressions() {
    std::string source = "ACTION calc { value := (10 + 5) * 2; }\n";
    std::string result = parseAndPrint(source);
    
    assert(result.find("ActionDecl (calc)") != std::string::npos);
    assert(result.find("AssignStmt (value :=)") != std::string::npos);
    assert(result.find("BinaryExpr (*)") != std::string::npos);
    assert(result.find("BinaryExpr (+)") != std::string::npos);
    std::cout << "testExpressions passed!" << std::endl;
}

void testBranchLoop() {
    std::string source = "FUNC play() { BRANCH (hero_health > 0) { ACTION attack {} } ELSE { ACTION run {} } }\n";
    std::string result = parseAndPrint(source);
    
    assert(result.find("FunctionDecl (play) Params: []") != std::string::npos);
    assert(result.find("BranchStmt") != std::string::npos);
    assert(result.find("Else:") != std::string::npos);
    std::cout << "testBranchLoop passed!" << std::endl;
}

int main() {
    std::cout << "Running Parser Tests...\n";
    
    testVarDecl();
    testStateDecl();
    testEventDecl();
    testExpressions();
    testBranchLoop();
    
    std::cout << "All Parser Tests Passed!\n";
    return 0;
}
