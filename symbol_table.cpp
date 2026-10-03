#include "symbol_table.h"

void SymbolTable::enterScope()
{
    scopes_.emplace_back();
}

void SymbolTable::exitScope()
{
    if (!scopes_.empty())
    {
        scopes_.pop_back();
    }
}

void SymbolTable::define(const Symbol &symbol)
{
    if (scopes_.empty())
    {
        scopes_.emplace_back();
    }
    scopes_.back()[symbol.name] = symbol;
}

Symbol *SymbolTable::lookup(const std::string &name)
{
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
        {
            return const_cast<Symbol *>(&found->second);
        }
    }
    return nullptr;
}

const Symbol *SymbolTable::lookup(const std::string &name) const
{
    for (auto it = scopes_.crbegin(); it != scopes_.crend(); ++it)
    {
        auto found = it->find(name);
        if (found != it->end())
        {
            return &found->second;
        }
    }
    return nullptr;
}

Symbol *SymbolTable::lookupLocal(const std::string &name)
{
    if (scopes_.empty())
    {
        return nullptr;
    }
    auto found = scopes_.back().find(name);
    return found == scopes_.back().end() ? nullptr : &found->second;
}

const Symbol *SymbolTable::lookupLocal(const std::string &name) const
{
    if (scopes_.empty())
    {
        return nullptr;
    }
    const auto &scope = scopes_.back();
    auto found = scope.find(name);
    return found == scope.end() ? nullptr : &found->second;
}

bool SymbolTable::isDefined(const std::string &name) const
{
    return lookup(name) != nullptr;
}
