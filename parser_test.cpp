#include "parser.h"
#include <cassert>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    void expect(bool condition, const std::string &message)
    {
        if (!condition)
        {
            throw std::runtime_error(message);
        }
    }

    void expectType(const std::unique_ptr<Program> &program, size_t expectedCount, const std::string &message)
    {
        expect(program != nullptr, message + ": program is null");
        expect(program->declarations.size() + program->statements.size() == expectedCount,
               message + ": unexpected total node count");
    }

    void test_simple_character_declaration()
    {
        Lexer lexer("CHAR alice: wanderer");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program != nullptr, "simple character declaration: program null");
        expect(program->declarations.size() == 1, "simple character declaration: declaration count");
        auto decl = dynamic_cast<CharDecl *>(program->declarations[0].get());
        expect(decl != nullptr, "simple character declaration: wrong declaration type");
        expect(decl->name == "alice", "simple character declaration: wrong name");
        expect(decl->type == "wanderer", "simple character declaration: wrong type");
        std::cout << "PASS: simple character declaration" << std::endl;
    }

    void test_state_declaration()
    {
        Lexer lexer("STATE door_status: {open | locked}");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->declarations.size() == 1, "state declaration: declaration count");
        auto decl = dynamic_cast<StateDecl *>(program->declarations[0].get());
        expect(decl != nullptr, "state declaration: wrong declaration type");
        expect(decl->name == "door_status", "state declaration: wrong name");
        expect(decl->values.size() == 2, "state declaration: wrong enum count");
        expect(decl->values[0] == "open" && decl->values[1] == "locked", "state declaration: wrong enum values");
        std::cout << "PASS: state declaration" << std::endl;
    }

    void test_action_statement()
    {
        Lexer lexer("ACTION walk(alice, store)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->statements.size() == 1, "action statement: statement count");
        auto stmt = dynamic_cast<ActionStmt *>(program->statements[0].get());
        expect(stmt != nullptr, "action statement: wrong statement type");
        expect(stmt->name == "walk", "action statement: wrong name");
        expect(stmt->args.size() == 2, "action statement: wrong arg count");
        expect(stmt->args[0]->kind == NodeKind::Identifier && stmt->args[1]->kind == NodeKind::Identifier,
               "action statement: args must be identifiers");
        std::cout << "PASS: action statement" << std::endl;
    }

    void test_branch_statement()
    {
        Lexer lexer("BRANCH door == locked: ACTION wait(alice)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->statements.size() == 1, "branch statement: statement count");
        auto stmt = dynamic_cast<BranchStmt *>(program->statements[0].get());
        expect(stmt != nullptr, "branch statement: wrong statement type");
        expect(stmt->thenBlock.size() == 1, "branch statement: then block size");
        expect(stmt->condition != nullptr, "branch statement: missing condition");
        std::cout << "PASS: branch statement" << std::endl;
    }

    void test_nested_branches()
    {
        Lexer lexer("BRANCH x > 0: BRANCH y > 0: ACTION inner(x) ELSE: ACTION outer(x)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        auto outer = dynamic_cast<BranchStmt *>(program->statements[0].get());
        expect(outer != nullptr, "nested branches: outer branch null");
        expect(outer->thenBlock.size() == 1, "nested branches: outer then block size");
        auto inner = dynamic_cast<BranchStmt *>(outer->thenBlock[0].get());
        expect(inner != nullptr, "nested branches: inner branch null");
        expect(inner->elseBlock.size() == 1, "nested branches: else block size");
        std::cout << "PASS: nested branches" << std::endl;
    }

    void test_loop_statement()
    {
        Lexer lexer("LOOP i < 10: ACTION step(i)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->statements.size() == 1, "loop statement: statement count");
        auto loop = dynamic_cast<LoopStmt *>(program->statements[0].get());
        expect(loop != nullptr, "loop statement: wrong statement type");
        expect(loop->body.size() == 1, "loop statement: wrong body size");
        expect(loop->condition != nullptr, "loop statement: missing condition");
        std::cout << "PASS: loop statement" << std::endl;
    }

    void test_assignment_statement()
    {
        Lexer lexer("alice.inventory := sword");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->statements.size() == 1, "assignment statement: statement count");
        auto stmt = dynamic_cast<AssignmentStmt *>(program->statements[0].get());
        expect(stmt != nullptr, "assignment statement: wrong statement type");
        expect(stmt->lvalue == "alice.inventory", "assignment statement: wrong lvalue");
        expect(stmt->value != nullptr, "assignment statement: missing value");
        std::cout << "PASS: assignment statement" << std::endl;
    }

    void test_function_declaration()
    {
        Lexer lexer("FUNC greet(name, title): ACTION say(name)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        expect(program && program->declarations.size() == 1, "function declaration: declaration count");
        auto fn = dynamic_cast<FuncDecl *>(program->declarations[0].get());
        expect(fn != nullptr, "function declaration: wrong declaration type");
        expect(fn->name == "greet", "function declaration: wrong name");
        expect(fn->params.size() == 2, "function declaration: wrong param count");
        expect(fn->body.size() == 1, "function declaration: wrong body size");
        std::cout << "PASS: function declaration" << std::endl;
    }

    void test_complex_expression_parsing()
    {
        Lexer lexer("BRANCH (x > 5 && y != 10) || z < 0: ACTION done(z)");
        Parser parser(lexer.tokenize());
        auto program = parser.parse();

        auto branch = dynamic_cast<BranchStmt *>(program->statements[0].get());
        expect(branch != nullptr, "complex expression: branch null");
        expect(branch->condition != nullptr, "complex expression: missing condition");
        std::cout << "PASS: complex expression parsing" << std::endl;
    }

    void test_unexpected_token()
    {
        Lexer lexer("ACTION");
        Parser parser(lexer.tokenize());

        try
        {
            parser.parse();
            throw std::runtime_error("unexpected token: parse should have failed");
        }
        catch (const std::runtime_error &)
        {
            std::cout << "PASS: unexpected token" << std::endl;
        }
    }

    void test_missing_closing_paren()
    {
        Lexer lexer("ACTION walk(alice");
        Parser parser(lexer.tokenize());

        try
        {
            parser.parse();
            throw std::runtime_error("missing closing paren: parse should have failed");
        }
        catch (const std::runtime_error &)
        {
            std::cout << "PASS: missing closing paren" << std::endl;
        }
    }

    void test_invalid_statement()
    {
        Lexer lexer("UNKNOWN thing");
        Parser parser(lexer.tokenize());

        try
        {
            parser.parse();
            throw std::runtime_error("invalid statement: parse should have failed");
        }
        catch (const std::runtime_error &)
        {
            std::cout << "PASS: invalid statement" << std::endl;
        }
    }

} // namespace

int main()
{
    try
    {
        test_simple_character_declaration();
        test_state_declaration();
        test_action_statement();
        test_branch_statement();
        test_nested_branches();
        test_loop_statement();
        test_assignment_statement();
        test_function_declaration();
        test_complex_expression_parsing();
        test_unexpected_token();
        test_missing_closing_paren();
        test_invalid_statement();

        std::cout << "ALL PARSER TESTS PASSED" << std::endl;
        return 0;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "PARSER TEST FAILED: " << ex.what() << std::endl;
        return 1;
    }
}
