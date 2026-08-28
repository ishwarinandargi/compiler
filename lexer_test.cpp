#include "token.h"
#include <iostream>
#include <cassert>

/**
 * Test suite for StoryLang lexer
 * Run with: g++ -std=c++17 lexer.cpp lexer_test.cpp -o lexer_test && ./lexer_test
 */

// Helper function to assert token properties
void assertToken(const Token &token, TokenType expectedType,
                 const std::string &expectedLexeme,
                 const std::string &testName)
{
    if (token.type != expectedType || token.lexeme != expectedLexeme)
    {
        std::cerr << "FAIL: " << testName << std::endl;
        std::cerr << "  Expected: " << tokenTypeToString(expectedType)
                  << " '" << expectedLexeme << "'" << std::endl;
        std::cerr << "  Got: " << token.typeName()
                  << " '" << token.lexeme << "'" << std::endl;
        exit(1);
    }
    std::cout << "PASS: " << testName << std::endl;
}

// Test 1: Simple character declaration
void test_characterDeclaration()
{
    std::string source = "CHAR alice: brave_wanderer";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    assert(tokens.size() >= 4);
    assertToken(tokens[0], TokenType::CHAR, "CHAR", "test_characterDeclaration: CHAR keyword");
    assertToken(tokens[1], TokenType::IDENTIFIER, "alice", "test_characterDeclaration: identifier");
    assertToken(tokens[2], TokenType::COLON, ":", "test_characterDeclaration: colon");
    assertToken(tokens[3], TokenType::IDENTIFIER, "brave_wanderer", "test_characterDeclaration: archetype");
}

// Test 2: Action statement
void test_actionStatement()
{
    std::string source = "ACTION challenge(alice, dragon)";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    assert(tokens.size() >= 7);
    assertToken(tokens[0], TokenType::ACTION, "ACTION", "test_actionStatement: ACTION keyword");
    assertToken(tokens[1], TokenType::IDENTIFIER, "challenge", "test_actionStatement: action name");
    assertToken(tokens[2], TokenType::LPAREN, "(", "test_actionStatement: lparen");
    assertToken(tokens[3], TokenType::IDENTIFIER, "alice", "test_actionStatement: first arg");
    assertToken(tokens[4], TokenType::COMMA, ",", "test_actionStatement: comma");
    assertToken(tokens[5], TokenType::IDENTIFIER, "dragon", "test_actionStatement: second arg");
    assertToken(tokens[6], TokenType::RPAREN, ")", "test_actionStatement: rparen");
}

// Test 3: Branch (conditional) statement
void test_branchStatement()
{
    std::string source = "BRANCH dragon.mood == angry:";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    assert(tokens.size() >= 6);
    assertToken(tokens[0], TokenType::BRANCH, "BRANCH", "test_branchStatement: BRANCH keyword");
    assertToken(tokens[1], TokenType::IDENTIFIER, "dragon", "test_branchStatement: object");
    assertToken(tokens[2], TokenType::DOT, ".", "test_branchStatement: dot");
    assertToken(tokens[3], TokenType::IDENTIFIER, "mood", "test_branchStatement: property");
    assertToken(tokens[4], TokenType::EQ, "==", "test_branchStatement: equality");
    assertToken(tokens[5], TokenType::IDENTIFIER, "angry", "test_branchStatement: literal");
}

// Test 4: State assignment
void test_stateAssignment()
{
    std::string source = "alice.inventory := sword";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    assert(tokens.size() >= 5);
    assertToken(tokens[0], TokenType::IDENTIFIER, "alice", "test_stateAssignment: object");
    assertToken(tokens[1], TokenType::DOT, ".", "test_stateAssignment: dot");
    assertToken(tokens[2], TokenType::IDENTIFIER, "inventory", "test_stateAssignment: property");
    assertToken(tokens[3], TokenType::ASSIGN, ":=", "test_stateAssignment: assignment");
    assertToken(tokens[4], TokenType::IDENTIFIER, "sword", "test_stateAssignment: value");
}

// Test 5: Numbers and literals
void test_numbersAndLiterals()
{
    std::string source = "PROP health: 100 PROP damage: 15.5 PROP name: \"Alice\"";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Find the number tokens
    bool foundInt = false, foundFloat = false, foundString = false;
    for (const auto &token : tokens)
    {
        if (token.lexeme == "100")
        {
            assertToken(token, TokenType::NUMBER, "100", "test_numbersAndLiterals: integer");
            foundInt = true;
        }
        if (token.lexeme == "15.5")
        {
            assertToken(token, TokenType::NUMBER, "15.5", "test_numbersAndLiterals: float");
            foundFloat = true;
        }
        if (token.lexeme == "\"Alice\"")
        {
            assertToken(token, TokenType::STRING, "\"Alice\"", "test_numbersAndLiterals: string");
            foundString = true;
        }
    }
    assert(foundInt && foundFloat && foundString);
}

// Test 6: String with escape sequences
void test_stringEscapeSequences()
{
    std::string source = R"(ACTION say("Hello\nWorld\t!"))";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto &token : tokens)
    {
        if (token.type == TokenType::STRING)
        {
            assert(token.value.find('\n') != std::string::npos);
            assert(token.value.find('\t') != std::string::npos);
            std::cout << "PASS: test_stringEscapeSequences: escape handling" << std::endl;
            return;
        }
    }
    assert(false && "String token not found");
}

// Test 7: Comments (single-line)
void test_singleLineComments()
{
    std::string source = R"(
        CHAR alice: wanderer  // This is a comment
        ACTION walk          // Another comment
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Should skip comments and only find CHAR, IDENTIFIER, ACTION, IDENTIFIER
    size_t count = 0;
    for (const auto &token : tokens)
    {
        if (token.type != TokenType::NEWLINE && token.type != TokenType::EOF_TOKEN)
        {
            count++;
        }
    }
    assert(count >= 4);
    std::cout << "PASS: test_singleLineComments: comment skipping" << std::endl;
}

// Test 8: Multi-line comments
void test_multiLineComments()
{
    std::string source = R"(
        CHAR alice: wanderer
        /* This is a
           multi-line comment
           spanning several lines */
        ACTION walk
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Should skip multi-line comment
    for (const auto &token : tokens)
    {
        // Should not find the word "multi" or "comment" as tokens
        assert(token.lexeme != "multi");
        assert(token.lexeme != "comment");
    }
    std::cout << "PASS: test_multiLineComments: multi-line comment skipping" << std::endl;
}

// Test 9: Comparison operators
void test_comparisonOperators()
{
    std::string source = "BRANCH x == 5 BRANCH y != 3 BRANCH z < 10 BRANCH a <= 20 BRANCH b > 15 BRANCH c >= 25";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int eqCount = 0, neCount = 0, ltCount = 0, leCount = 0, gtCount = 0, geCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::EQ)
            eqCount++;
        if (token.type == TokenType::NE)
            neCount++;
        if (token.type == TokenType::LT)
            ltCount++;
        if (token.type == TokenType::LE)
            leCount++;
        if (token.type == TokenType::GT)
            gtCount++;
        if (token.type == TokenType::GE)
            geCount++;
    }

    assert(eqCount == 1 && neCount == 1 && ltCount == 1 && leCount == 1 && gtCount == 1 && geCount == 1);
    std::cout << "PASS: test_comparisonOperators: all operators recognized" << std::endl;
}

// Test 10: Logical operators
void test_logicalOperators()
{
    std::string source = "BRANCH (x && y) BRANCH (a || b) BRANCH !flag";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int andCount = 0, orCount = 0, notCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::AND)
            andCount++;
        if (token.type == TokenType::OR)
            orCount++;
        if (token.type == TokenType::NOT)
            notCount++;
    }

    assert(andCount == 1 && orCount == 1 && notCount == 1);
    std::cout << "PASS: test_logicalOperators: all operators recognized" << std::endl;
}

// Test 11: Arrow operators
void test_arrowOperators()
{
    std::string source = "INIT alice.location -> store FUNC result -> success";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int arrowCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::ARROW)
        {
            arrowCount++;
            assert(token.lexeme == "->");
        }
    }
    assert(arrowCount == 2);
    std::cout << "PASS: test_arrowOperators: arrow operator recognized" << std::endl;
}

// Test 12: Complex story example
void test_complexStory()
{
    std::string source = R"(
        CHAR alice: brave_wanderer
        CHAR dragon: guardian
        CHAR bob: merchant
        
        STATE door_status: {open, locked}
        
        INIT alice.inventory := sword
        INIT dragon.mood := calm
        
        EVENT alice_challenges_dragon:
            ACTION challenge(alice, dragon)
            TRIGGER dragon.mood := angry
        
        BRANCH dragon.mood == angry:
            ACTION fight(alice, dragon)
            BRANCH rand() > 0.5:
                INIT alice.status := victorious
                ACTION leave(alice)
            ELSE:
                INIT alice.status := fallen
                ACTION laugh(bob)
    )";

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    // Verify we got reasonable number of tokens
    assert(tokens.size() > 50);

    // Verify key tokens are present
    bool hasChar = false, hasState = false, hasBranch = false, hasEvent = false;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::CHAR)
            hasChar = true;
        if (token.type == TokenType::STATE)
            hasState = true;
        if (token.type == TokenType::BRANCH)
            hasBranch = true;
        if (token.type == TokenType::EVENT)
            hasEvent = true;
    }

    assert(hasChar && hasState && hasBranch && hasEvent);
    std::cout << "PASS: test_complexStory: complex story tokenized correctly" << std::endl;
    std::cout << "  Total tokens: " << tokens.size() << std::endl;
}

// Test 13: Error handling - unterminated string
void test_unterminatedString()
{
    std::string source = R"(ACTION say("Hello)";
    Lexer lexer(source);

    try
    {
        auto tokens = lexer.tokenize();
        assert(false && "Should have thrown on unterminated string");
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "PASS: test_unterminatedString: error caught" << std::endl;
        std::cout << "  Error message: " << e.what() << std::endl;
    }
}

// Test 14: Error handling - invalid character
void test_invalidCharacter()
{
    std::string source = "ACTION walk @ store";
    Lexer lexer(source);

    try
    {
        auto tokens = lexer.tokenize();
        assert(false && "Should have thrown on invalid character");
    }
    catch (const std::runtime_error &e)
    {
        std::cout << "PASS: test_invalidCharacter: error caught" << std::endl;
        std::cout << "  Error message: " << e.what() << std::endl;
    }
}

// Test 15: Character literals
void test_characterLiterals()
{
    std::string source = "CONST start: 'a' CONST end: 'z'";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int charCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::STRING && token.lexeme.length() == 3 &&
            token.lexeme[0] == '\'' && token.lexeme[2] == '\'')
        {
            charCount++;
        }
    }

    assert(charCount >= 2);
    std::cout << "PASS: test_characterLiterals: character literals recognized" << std::endl;
}

// Test 16: Line tracking
void test_lineTracking()
{
    std::string source = "CHAR alice: wanderer\nACTION walk\nACTION sit";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int line1Count = 0, line2Count = 0, line3Count = 0;
    for (const auto &token : tokens)
    {
        if (token.line == 1 && token.type != TokenType::NEWLINE)
            line1Count++;
        if (token.line == 2 && token.type != TokenType::NEWLINE)
            line2Count++;
        if (token.line == 3 && token.type != TokenType::NEWLINE)
            line3Count++;
    }

    assert(line1Count > 0 && line2Count > 0 && line3Count > 0);
    std::cout << "PASS: test_lineTracking: line numbers tracked correctly" << std::endl;
    std::cout << "  Line 1: " << line1Count << " tokens, Line 2: " << line2Count
              << " tokens, Line 3: " << line3Count << " tokens" << std::endl;
}

// Test 17: Scientific notation
void test_scientificNotation()
{
    std::string source = "CONST e1: 1e5 CONST e2: 2.5e-3 CONST e3: 3.14E+2";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    int sciCount = 0;
    for (const auto &token : tokens)
    {
        if (token.type == TokenType::NUMBER &&
            (token.lexeme.find('e') != std::string::npos ||
             token.lexeme.find('E') != std::string::npos))
        {
            sciCount++;
        }
    }

    assert(sciCount >= 3);
    std::cout << "PASS: test_scientificNotation: scientific notation recognized" << std::endl;
}

// Main test runner
int main()
{
    std::cout << "=== StoryLang Lexer Test Suite ===" << std::endl
              << std::endl;

    try
    {
        test_characterDeclaration();
        test_actionStatement();
        test_branchStatement();
        test_stateAssignment();
        test_numbersAndLiterals();
        test_stringEscapeSequences();
        test_singleLineComments();
        test_multiLineComments();
        test_comparisonOperators();
        test_logicalOperators();
        test_arrowOperators();
        test_complexStory();
        test_unterminatedString();
        test_invalidCharacter();
        test_characterLiterals();
        test_lineTracking();
        test_scientificNotation();

        std::cout << "\n=== ALL TESTS PASSED ===" << std::endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "\n!!! TEST FAILED !!!" << std::endl;
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}