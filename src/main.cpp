// ============================================================
//  MINI COMPILER – Main Driver
//
//  Demonstrates the full compiler pipeline:
//    1. Lexical Analysis   (Lexer)
//    2. Parsing            (Parser)
//    3. AST Construction   (via Parser)
//    4. Symbol Table
//    5. Interpretation     (Interpreter)
//    6. IR Generation      (IRGenerator)
//    7. Optimisation       (Optimizer)
//
//  GPU / LLVM Connection
//  ---------------------
//  The LLVM-style IR printed by IRGenerator directly mirrors the
//  format used by real GPU compiler toolchains (NVPTX, AMDGPU)
//  which accept LLVM IR as input.  Our "mul i32" / "add i32"
//  instructions would translate 1-to-1 to PTX/SASS operations
//  on NVIDIA hardware.
// ============================================================

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symbol_table.h"
#include "interpreter.h"
#include "ir_generator.h"
#include "optimizer.h"

// ---- Divider helper for pretty output ----------------------
static void section(const std::string& title) {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║  " << title;
    // Pad to 54 chars
    int pad = 52 - static_cast<int>(title.size());
    if (pad > 0) std::cout << std::string(pad, ' ');
    std::cout << "║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
}

// ---- Print token list --------------------------------------
static void printTokens(const std::vector<Token>& tokens) {
    std::cout << "Tokens (" << tokens.size() << "):\n";
    for (auto& t : tokens) {
        if (t.type == TokenType::END_OF_FILE) break;
        std::cout << "  " << t.toString() << "\n";
    }
}

// ---- Print optimised IR ------------------------------------
static void printOptimizedIR(const std::vector<IRInstruction>& ir) {
    std::cout << "Optimized IR:\n";
    for (auto& instr : ir) {
        if (instr.op == "branch_if")
            std::cout << "  if " << instr.arg1 << " goto THEN\n";
        else if (instr.op == "branch_else")
            std::cout << "  goto END; ELSE:\n";
        else if (instr.op == "branch_end")
            std::cout << "  END:\n";
        else
            std::cout << "  " << instr.toTAC() << "\n";
    }
}

// ---- Run one test case -------------------------------------
static void runTest(const std::string& label, const std::string& src) {
    std::cout << "\n\n";
    std::cout << "══════════════════════════════════════════════════════\n";
    std::cout << "  TEST: " << label << "\n";
    std::cout << "  Source: " << src << "\n";
    std::cout << "══════════════════════════════════════════════════════\n";

    try {
        // ── PHASE 1: LEXER ──────────────────────────────────────
        section("PHASE 1 — LEXICAL ANALYSIS");
        Lexer lexer(src);
        std::vector<Token> tokens = lexer.tokenize();
        printTokens(tokens);

        // ── PHASE 2 + 3: PARSER + AST ───────────────────────────
        section("PHASE 2+3 — PARSER & AST");
        Parser parser(tokens);
        ASTNodePtr ast = parser.parse();
        ast->print();

        // ── PHASE 4 + 5: SYMBOL TABLE + INTERPRETER ─────────────
        section("PHASE 4+5 — SYMBOL TABLE & INTERPRETER");
        SymbolTable symTable;
        Interpreter interp(symTable);
        interp.run(ast.get());
        symTable.dump();

        // ── PHASE 6: IR GENERATION ───────────────────────────────
        section("PHASE 6 — IR GENERATION");
        IRGenerator irGen;
        irGen.generate(ast.get());
        irGen.printTAC();
        std::cout << "\n";
        irGen.printLLVM();

        // ── PHASE 7: OPTIMIZATION ────────────────────────────────
        section("PHASE 7 — OPTIMIZATION PASSES");
        Optimizer opt;
        auto optimized = opt.optimize(irGen.instructions());
        printOptimizedIR(optimized);
        std::cout << "\n";
        opt.printStats();

    } catch (const std::exception& ex) {
        std::cerr << "\n[COMPILER ERROR] " << ex.what() << "\n";
    }
}

// ============================================================
//  main
// ============================================================
int main() {
    
    std::cout << "STARTING PROGRAM...\n";
    std::cout << "HELLO\n";
std::cout.flush();
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║          MINI C-LIKE COMPILER  v1.0                 ║\n";
    std::cout << "║    Lexer → Parser → AST → IR → Optimizer            ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";

    // ── TEST 1: The canonical example from the spec ──────────────
    runTest("Canonical spec example",
            "a = 2 + 3 * 4;");

    // ── TEST 2: Multiple dependent assignments ───────────────────
    runTest("Multiple dependent assignments",
            "a = 5; b = a + 2; c = b * 3;");

    // ── TEST 3: Parentheses override precedence ──────────────────
    runTest("Parentheses override precedence",
            "a = (2 + 3) * 4;");

    // ── TEST 4: Constant folding bait ────────────────────────────
    runTest("Constant folding: all literals",
            "x = 10 + 5 * 2; y = x + 0; z = y * 1;");

    // ── TEST 5: Dead code elimination ───────────────────────────
    runTest("Dead code: overwritten variable",
            "a = 5; a = 10; b = a + 1;");

    // ── TEST 6: Complex arithmetic ───────────────────────────────
    runTest("Complex arithmetic chain",
            "a = 100; b = a / 5; c = b * (3 + 7);");

    // ── TEST 7: if statement (optional extension) ────────────────
    runTest("If statement (optional extension)",
            "x = 10; if (x > 5) { y = 1; } else { y = 0; }");

    // ── TEST 8: Identity simplifications ─────────────────────────
    runTest("Identity simplifications x*1, x+0, x*0",
            "a = 7; b = a * 1; c = b + 0; d = c * 0;");

    // ── TEST 9: Error handling demo (undefined variable) ─────────
    runTest("Error: undefined variable",
            "b = z + 1;");

    // ── TEST 10: Error handling demo (missing semicolon) ─────────
    runTest("Error: missing semicolon",
            "a = 5 b = 10;");

    std::cout << "\n\n╔══════════════════════════════════════════════════════╗\n";
    std::cout <<     "║              All tests complete.                    ║\n";
    std::cout <<     "╚══════════════════════════════════════════════════════╝\n\n";

    return 0;
}
