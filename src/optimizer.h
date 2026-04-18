#pragma once
// ============================================================
//  OPTIMIZER
//  Phase 7 of the compiler pipeline.
//
//  Operates on the flat IR instruction list produced by
//  IRGenerator and applies several classic optimisation passes:
//
//  Pass 1 – Constant Folding
//    Replace binary operations whose both operands are numeric
//    literals with a single literal.
//    e.g.  t1 = 3 * 4   →  t1 = 12
//
//  Pass 2 – Copy Propagation
//    If an instruction is a simple copy  (dest = src)  and src
//    is a known constant, substitute the constant wherever dest
//    is used later.
//    e.g.  t2 = 2 + t1   (after folding t1=12)
//          →  t2 = 2 + 12   →  (then fold again)  t2 = 14
//
//  Pass 3 – Dead Code Elimination
//    Remove assignments to a variable that are subsequently
//    overwritten before any use.
//    e.g.  a = 5   (dead if next is  a = 10)
//
//  Pass 4 – Identity Simplification
//    x * 1  →  x
//    x + 0  →  x
//    x - 0  →  x
//    x * 0  →  0
//    x / 1  →  x
// ============================================================

#include "ir_generator.h"
#include <vector>
#include <string>

class Optimizer {
public:
    // Run all passes on a copy of the instruction list and return
    // the optimised version.
    std::vector<IRInstruction> optimize(const std::vector<IRInstruction>& ir);

    void printStats() const;

private:
    int foldCount_      = 0;
    int deadCount_      = 0;
    int identityCount_  = 0;
    int propCount_      = 0;

    // Individual passes (mutate the vector in-place)
    void passIdentitySimplification(std::vector<IRInstruction>& ir);
    void passConstantFolding       (std::vector<IRInstruction>& ir);
    void passCopyPropagation       (std::vector<IRInstruction>& ir);
    void passDeadCodeElimination   (std::vector<IRInstruction>& ir);

    // Utility: is this string a numeric literal?
    static bool isNumber(const std::string& s);
    static int  toInt   (const std::string& s);
    static int  applyOp (const std::string& op, int a, int b);
};
