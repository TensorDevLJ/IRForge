#pragma once
// ============================================================
//  INTERPRETER  (AST tree-walking evaluator)
//  Phase 5 of the compiler pipeline.
//
//  Walks the AST recursively and evaluates every expression.
//  Variable state is maintained in the SymbolTable.
// ============================================================

#include "ast.h"
#include "symbol_table.h"

class Interpreter {
public:
    explicit Interpreter(SymbolTable& symTable);

    // Execute a complete program (ProgramNode)
    void run(const ASTNode* node);

private:
    SymbolTable& sym_;

    // Evaluate any expression node and return its integer value
    int eval(const ASTNode* node);
};
