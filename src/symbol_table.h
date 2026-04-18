#pragma once
// ============================================================
//  SYMBOL TABLE
//  Phase 4 of the compiler pipeline.
//
//  A flat, single-scope symbol table backed by an unordered_map.
//  In a full compiler this would grow into a stack of scopes,
//  type information per symbol, etc.
// ============================================================

#include <string>
#include <unordered_map>
#include <stdexcept>
#include <iostream>

class SymbolTable {
public:
    // Store or update a variable
    void set(const std::string& name, int value) {
        table_[name] = value;
    }

    // Retrieve a variable; throws if undeclared
    int get(const std::string& name) const {
        auto it = table_.find(name);
        if (it == table_.end())
            throw std::runtime_error(
                "[Runtime Error] Undefined variable: '" + name + "'");
        return it->second;
    }

    // Check existence without throwing
    bool has(const std::string& name) const {
        return table_.count(name) > 0;
    }

    // Dump the entire table (used for debug output)
    void dump() const {
        std::cout << "Symbol Table:\n";
        for (auto& [k, v] : table_)
            std::cout << "  " << k << " = " << v << "\n";
    }

    // Expose the underlying map (needed by the IR optimiser)
    const std::unordered_map<std::string, int>& data() const { return table_; }

private:
    std::unordered_map<std::string, int> table_;
};
