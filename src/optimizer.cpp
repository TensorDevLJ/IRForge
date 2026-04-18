// ============================================================
//  OPTIMIZER – implementation
// ============================================================
#include "optimizer.h"
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <stdexcept>
#include <cctype>

// ---- static helpers ----------------------------------------
bool Optimizer::isNumber(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    if (start == s.size()) return false;
    for (size_t i = start; i < s.size(); ++i)
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    return true;
}

int Optimizer::toInt(const std::string& s) { return std::stoi(s); }

int Optimizer::applyOp(const std::string& op, int a, int b) {
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "*") return a * b;
    if (op == "/") {
        if (b == 0) throw std::runtime_error("[Optimizer] Division by zero during folding");
        return a / b;
    }
    if (op == ">") return a > b ? 1 : 0;
    if (op == "<") return a < b ? 1 : 0;
    throw std::runtime_error("[Optimizer] Unknown operator: " + op);
}

// ---- Pass 1: Identity Simplification -----------------------
//   x * 1 → x,  x + 0 → x,  x - 0 → x,  x * 0 → 0,  x / 1 → x
void Optimizer::passIdentitySimplification(std::vector<IRInstruction>& ir) {
    for (auto& i : ir) {
        if (i.op == "=" || i.arg2.empty()) continue;

        // x * 1  →  x
        if (i.op == "*" && i.arg2 == "1") {
            i.op = "="; i.arg2 = ""; ++identityCount_;
        }
        // x + 0  →  x
        else if (i.op == "+" && i.arg2 == "0") {
            i.op = "="; i.arg2 = ""; ++identityCount_;
        }
        // x - 0  →  x
        else if (i.op == "-" && i.arg2 == "0") {
            i.op = "="; i.arg2 = ""; ++identityCount_;
        }
        // x * 0  →  0
        else if (i.op == "*" && i.arg2 == "0") {
            i.op = "="; i.arg1 = "0"; i.arg2 = ""; ++identityCount_;
        }
        // 0 * x  →  0
        else if (i.op == "*" && i.arg1 == "0") {
            i.op = "="; i.arg2 = ""; ++identityCount_;
        }
        // x / 1  →  x
        else if (i.op == "/" && i.arg2 == "1") {
            i.op = "="; i.arg2 = ""; ++identityCount_;
        }
        // 1 * x  →  x
        else if (i.op == "*" && i.arg1 == "1") {
            i.op = "="; i.arg1 = i.arg2; i.arg2 = ""; ++identityCount_;
        }
        // x + 0 when 0 is on left
        else if (i.op == "+" && i.arg1 == "0") {
            i.op = "="; i.arg1 = i.arg2; i.arg2 = ""; ++identityCount_;
        }
    }
}

// ---- Pass 2: Constant Folding ------------------------------
void Optimizer::passConstantFolding(std::vector<IRInstruction>& ir) {
    for (auto& i : ir) {
        // Only fold proper binary ops where BOTH args are literals
        if (i.op == "=" || i.arg2.empty()) continue;
        if (isNumber(i.arg1) && isNumber(i.arg2)) {
            int result = applyOp(i.op, toInt(i.arg1), toInt(i.arg2));
            i.op   = "=";
            i.arg1 = std::to_string(result);
            i.arg2 = "";
            ++foldCount_;
        }
    }
}

// ---- Pass 3: Copy Propagation ------------------------------
//  Forward-substitute known constant or temp values
void Optimizer::passCopyPropagation(std::vector<IRInstruction>& ir) {
    // Map from name → constant string (only propagate constants for safety)
    std::unordered_map<std::string, std::string> constMap;

    for (auto& i : ir) {
        // Substitute arg1 and arg2
        auto sub = [&](std::string& arg) {
            if (!arg.empty() && !isNumber(arg)) {
                auto it = constMap.find(arg);
                if (it != constMap.end()) {
                    arg = it->second;
                    ++propCount_;
                }
            }
        };
        sub(i.arg1);
        if (!i.arg2.empty()) sub(i.arg2);

        // Record what this instruction defines
        if (!i.result.empty() && i.op == "=" && isNumber(i.arg1)) {
            constMap[i.result] = i.arg1;   // result is now a known constant
        } else {
            // The result is recomputed; forget any previous binding
            constMap.erase(i.result);
        }
    }
}

// ---- Pass 4: Dead Code Elimination -------------------------
//  Remove an assignment if the same variable is assigned again
//  later with no intervening reads.
void Optimizer::passDeadCodeElimination(std::vector<IRInstruction>& ir) {
    // Build a set of "dead" instruction indices
    std::unordered_set<size_t> dead;

    for (size_t i = 0; i < ir.size(); ++i) {
        if (ir[i].result.empty() || ir[i].op == "") continue;

        const std::string& target = ir[i].result;

        // Look for a later instruction that writes to target before any read
        bool laterWrite = false, laterRead = false;
        for (size_t j = i + 1; j < ir.size(); ++j) {
            // Read check: target appears as arg
            if (ir[j].arg1 == target || ir[j].arg2 == target) {
                laterRead = true; break;
            }
            // Write check: later instruction redefines target
            if (ir[j].result == target) {
                laterWrite = true; break;
            }
        }
        if (laterWrite && !laterRead) {
            dead.insert(i);
            ++deadCount_;
        }
    }

    // Erase dead instructions (back-to-front to preserve indices)
    for (int k = static_cast<int>(ir.size()) - 1; k >= 0; --k) {
        if (dead.count(static_cast<size_t>(k)))
            ir.erase(ir.begin() + k);
    }
}

// ---- Main entry point: run all passes ----------------------
std::vector<IRInstruction> Optimizer::optimize(const std::vector<IRInstruction>& ir) {
    std::vector<IRInstruction> opt = ir;   // working copy

    // Run multiple rounds until no change (fixed-point iteration)
    for (int round = 0; round < 5; ++round) {
        size_t before = opt.size();
        int prevFold  = foldCount_;

        passIdentitySimplification(opt);
        passConstantFolding(opt);
        passCopyPropagation(opt);
        passConstantFolding(opt);   // second pass after propagation
        passDeadCodeElimination(opt);

        // Stop if nothing changed this round
        if (opt.size() == before && foldCount_ == prevFold) break;
    }

    return opt;
}

// ---- Print optimization statistics -------------------------
void Optimizer::printStats() const {
    std::cout << "Optimization Stats:\n";
    std::cout << "  Constant folds       : " << foldCount_     << "\n";
    std::cout << "  Copy propagations    : " << propCount_     << "\n";
    std::cout << "  Dead instructions    : " << deadCount_     << "\n";
    std::cout << "  Identity simplifications: " << identityCount_ << "\n";
}
