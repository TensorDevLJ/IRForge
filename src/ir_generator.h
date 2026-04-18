#pragma once
// ============================================================
//  IR GENERATOR  (Intermediate Representation)
//  Phase 6 of the compiler pipeline.
//
//  Converts the AST into Three-Address Code (TAC) and also
//  emits a corresponding LLVM-style SSA IR for comparison.
//
//  TAC form:
//    t1 = 3 * 4
//    t2 = 2 + t1
//    a  = t2
//
//  LLVM-style form:
//    %1 = mul i32 3, 4
//    %2 = add i32 2, %1
//    store i32 %2, ptr @a
// ============================================================

#include "ast.h"
#include <string>
#include <vector>

// A single TAC instruction
struct IRInstruction {
    std::string result;   // destination  (e.g. "t1", "a")
    std::string op;       // operator     (e.g. "+", "-", "*", "/", "=")
    std::string arg1;     // left operand
    std::string arg2;     // right operand (empty for copy / store)

    // Human-readable TAC string
    std::string toTAC() const;

    // Human-readable LLVM-style string
    std::string toLLVM() const;
};

class IRGenerator {
public:
    IRGenerator();

    // Generate IR for a whole program; result is in instructions()
    void generate(const ASTNode* node);

    const std::vector<IRInstruction>& instructions() const { return instrs_; }

    void printTAC()  const;
    void printLLVM() const;

private:
    std::vector<IRInstruction> instrs_;
    int                        tempCount_; 
    int labelCount_;  // counter for t1, t2, …

    std::string newTemp();    // allocate next temporary name
    std::string newLabel();
    // Recursively lower an expression; returns the name holding its value
    std::string lowerExpr(const ASTNode* node);

    // Lower a single statement
    void lowerStatement(const ASTNode* node);
};
