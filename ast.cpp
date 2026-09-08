#include "ast.h"
#include <sstream>

std::string ASTPrinter::print(ProgramAST& program) {
    result.clear();
    isLastChildStack.clear();
    program.accept(*this);
    return result;
}

void ASTPrinter::printIndent() {
    if (isLastChildStack.empty()) return;

    for (size_t i = 0; i < isLastChildStack.size() - 1; ++i) {
        if (isLastChildStack[i]) {
            result += "    ";
        } else {
            result += "│   ";
        }
    }
    
    if (isLastChildStack.back()) {
        result += "└── ";
    } else {
        result += "├── ";
    }
}

void ASTPrinter::appendLine(const std::string& text) {
    printIndent();
    result += text + "\n";
}

void ASTPrinter::visitNumberExpr(NumberExpr& node) {
    appendLine("NumberExpr (" + std::to_string(node.value) + ")");
}

void ASTPrinter::visitStringExpr(StringExpr& node) {
    appendLine("StringExpr (\"" + node.value + "\")");
}

void ASTPrinter::visitIdentifierExpr(IdentifierExpr& node) {
    appendLine("IdentifierExpr (" + node.name.lexeme + ")");
}

void ASTPrinter::visitBinaryExpr(BinaryExpr& node) {
    appendLine("BinaryExpr (" + node.op.lexeme + ")");
    isLastChildStack.push_back(false);
    node.left->accept(*this);
    isLastChildStack.pop_back();
    
    isLastChildStack.push_back(true);
    node.right->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitUnaryExpr(UnaryExpr& node) {
    appendLine("UnaryExpr (" + node.op.lexeme + ")");
    isLastChildStack.push_back(true);
    node.right->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitCallExpr(CallExpr& node) {
    appendLine("CallExpr");
    isLastChildStack.push_back(node.arguments.empty());
    node.callee->accept(*this);
    isLastChildStack.pop_back();
    
    for (size_t i = 0; i < node.arguments.size(); ++i) {
        isLastChildStack.push_back(i == node.arguments.size() - 1);
        node.arguments[i]->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitBlockStmt(BlockStmt& node) {
    appendLine("BlockStmt");
    for (size_t i = 0; i < node.statements.size(); ++i) {
        isLastChildStack.push_back(i == node.statements.size() - 1);
        node.statements[i]->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitExprStmt(ExprStmt& node) {
    appendLine("ExprStmt");
    isLastChildStack.push_back(true);
    node.expression->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitVarDeclStmt(VarDeclStmt& node) {
    appendLine("VarDeclStmt (" + node.typeKeyword.lexeme + " " + node.name.lexeme + ")");
    if (node.initializer) {
        isLastChildStack.push_back(true);
        node.initializer->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitAssignStmt(AssignStmt& node) {
    appendLine("AssignStmt (" + node.name.lexeme + " :=)");
    isLastChildStack.push_back(true);
    node.value->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitBranchStmt(BranchStmt& node) {
    appendLine("BranchStmt");
    
    isLastChildStack.push_back(false);
    appendLine("Condition:");
    isLastChildStack.push_back(true);
    node.condition->accept(*this);
    isLastChildStack.pop_back();
    isLastChildStack.pop_back();
    
    isLastChildStack.push_back(node.elseBranch == nullptr);
    appendLine("Then:");
    isLastChildStack.push_back(true);
    node.thenBranch->accept(*this);
    isLastChildStack.pop_back();
    isLastChildStack.pop_back();
    
    if (node.elseBranch) {
        isLastChildStack.push_back(true);
        appendLine("Else:");
        isLastChildStack.push_back(true);
        node.elseBranch->accept(*this);
        isLastChildStack.pop_back();
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitLoopStmt(LoopStmt& node) {
    appendLine("LoopStmt");
    
    isLastChildStack.push_back(false);
    appendLine("Condition:");
    isLastChildStack.push_back(true);
    node.condition->accept(*this);
    isLastChildStack.pop_back();
    isLastChildStack.pop_back();
    
    isLastChildStack.push_back(true);
    appendLine("Body:");
    isLastChildStack.push_back(true);
    node.body->accept(*this);
    isLastChildStack.pop_back();
    isLastChildStack.pop_back();
}

void ASTPrinter::visitReturnStmt(ReturnStmt& node) {
    appendLine("ReturnStmt");
    if (node.value) {
        isLastChildStack.push_back(true);
        node.value->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitFunctionDecl(FunctionDecl& node) {
    std::string params = "";
    for (size_t i = 0; i < node.parameters.size(); ++i) {
        params += node.parameters[i].lexeme;
        if (i < node.parameters.size() - 1) params += ", ";
    }
    appendLine("FunctionDecl (" + node.name.lexeme + ") Params: [" + params + "]");
    isLastChildStack.push_back(true);
    node.body->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitEventDecl(EventDecl& node) {
    std::string params = "";
    for (size_t i = 0; i < node.parameters.size(); ++i) {
        params += node.parameters[i].lexeme;
        if (i < node.parameters.size() - 1) params += ", ";
    }
    appendLine("EventDecl (" + node.name.lexeme + ") Params: [" + params + "]");
    isLastChildStack.push_back(true);
    node.body->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitStateDecl(StateDecl& node) {
    appendLine("StateDecl (" + node.name.lexeme + ")");
    if (node.initializer) {
        isLastChildStack.push_back(true);
        node.initializer->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitActionDecl(ActionDecl& node) {
    appendLine("ActionDecl (" + node.name.lexeme + ")");
    isLastChildStack.push_back(true);
    node.body->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitPropDecl(PropDecl& node) {
    appendLine("PropDecl (" + node.name.lexeme + ")");
    if (node.initializer) {
        isLastChildStack.push_back(true);
        node.initializer->accept(*this);
        isLastChildStack.pop_back();
    }
}

void ASTPrinter::visitInitDecl(InitDecl& node) {
    appendLine("InitDecl");
    isLastChildStack.push_back(true);
    node.body->accept(*this);
    isLastChildStack.pop_back();
}

void ASTPrinter::visitProgramAST(ProgramAST& node) {
    appendLine("ProgramAST");
    for (size_t i = 0; i < node.declarations.size(); ++i) {
        isLastChildStack.push_back(i == node.declarations.size() - 1);
        node.declarations[i]->accept(*this);
        isLastChildStack.pop_back();
    }
}
