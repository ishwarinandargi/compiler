#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

struct Symbol
{
    enum class Kind
    {
        Char,
        State,
        Func,
        Var
    };

    std::string name;
    Kind kind = Kind::Var;
    std::string dataType;
    std::string value;
    int line = 0;
    int column = 0;
    bool initialized = false;
    std::vector<Symbol> parameters;
    std::map<std::string, std::string> attributes;
};

class SymbolTable
{
public:
    void enterScope();
    void exitScope();
    void define(const Symbol &symbol);
    Symbol *lookup(const std::string &name);
    const Symbol *lookup(const std::string &name) const;
    Symbol *lookupLocal(const std::string &name);
    const Symbol *lookupLocal(const std::string &name) const;
    bool isDefined(const std::string &name) const;

    const std::vector<std::map<std::string, Symbol>> &scopes() const { return scopes_; }

private:
    std::vector<std::map<std::string, Symbol>> scopes_;
};
