// ============================================================
//  IR GENERATOR – implementation
// ============================================================
#include "ir_generator.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// ---- IRInstruction helpers ---------------------------------

static std::string llvmOp(const std::string& op) {
    if (op == "+") return "add";
    if (op == "-") return "sub";
    if (op == "*") return "mul";
    if (op == "/") return "sdiv";
    if (op == ">") return "icmp sgt";
    if (op == "<") return "icmp slt";
    return "???";
}

std::string IRInstruction::toTAC() const {
    if (op == "=")   // simple copy / final store
        return result + " = " + arg1;
    return result + " = " + arg1 + " " + op + " " + arg2;
}

std::string IRInstruction::toLLVM() const {
    // Determine whether result is a temp (%N) or a named global (@name)
    bool isTemp = (!result.empty() && result[0] == 't' && result.size() > 1);
    std::string llvmDest = isTemp ? ("%" + result.substr(1)) : ("@" + result);

    if (op == "=") {
        // store into variable — format the source value
        std::string src;
        if (!arg1.empty() && arg1[0] == 't' && arg1.size() > 1 &&
            std::isdigit(static_cast<unsigned char>(arg1[1])))
            src = "%" + arg1.substr(1);   // temp register  e.g. t2 -> %2
        else
            src = arg1;                   // literal number or variable name
        return "store i32 " + src + ", ptr " + llvmDest;
    }
    // binary op: dest = op i32 arg1, arg2
    auto fmtArg = [](const std::string& a) -> std::string {
        if (!a.empty() && a[0] == 't') return "%" + a.substr(1);
        return a;   // literal number
    };
    return llvmDest + " = " + llvmOp(op) +
           " i32 " + fmtArg(arg1) + ", " + fmtArg(arg2);
}

// ---- IRGenerator -------------------------------------------
IRGenerator::IRGenerator() : tempCount_(0), labelCount_(0) {}

std::string IRGenerator::newLabel() {
    return "L" + std::to_string(++labelCount_);
}

std::string IRGenerator::newTemp() {
    return "t" + std::to_string(++tempCount_);
}
// ---- Generate for the whole program ------------------------
void IRGenerator::generate(const ASTNode* node) {
    if (!node) return;
    if (node->type == ASTNodeType::PROGRAM) {
        const auto* prog = static_cast<const ProgramNode*>(node);
        for (auto& stmt : prog->statements)
            lowerStatement(stmt.get());
        return;
    }
    lowerStatement(node);
}

// ---- Lower a single statement ------------------------------
void IRGenerator::lowerStatement(const ASTNode* node) {
    if (node->type == ASTNodeType::ASSIGN) {
        const auto* asgn = static_cast<const AssignNode*>(node);
        std::string src = lowerExpr(asgn->expr.get());
        // Emit final store:  varName = src
        instrs_.push_back({asgn->name, "=", src, ""});
        return;
    }

  if (node->type == ASTNodeType::IF_STMT) {
    const auto* ifn = static_cast<const IfNode*>(node);

    std::string condTmp = lowerExpr(ifn->condition.get());

    std::string labelThen = newLabel();
    std::string labelElse = newLabel();
    std::string labelEnd  = newLabel();

    instrs_.push_back({"", "if_goto", condTmp, labelThen});
    instrs_.push_back({"", "goto", "", labelElse});

    instrs_.push_back({labelThen, "label", "", ""});
    for (auto& s : ifn->thenBody)
        lowerStatement(s.get());

    instrs_.push_back({"", "goto", "", labelEnd});

    instrs_.push_back({labelElse, "label", "", ""});
    for (auto& s : ifn->elseBody)
        lowerStatement(s.get());

    instrs_.push_back({labelEnd, "label", "", ""});

    return;
}

    throw std::runtime_error("[IRGen] Unknown statement node type");
}

// ---- Lower an expression; return the name/temp holding value
std::string IRGenerator::lowerExpr(const ASTNode* node) {
    switch (node->type) {
        case ASTNodeType::NUMBER:
            // Return the literal as a string; no instruction emitted
            return std::to_string(static_cast<const NumberNode*>(node)->value);

        case ASTNodeType::IDENTIFIER:
            return static_cast<const IdentifierNode*>(node)->name;

        case ASTNodeType::BINARY_OP: {
            const auto* bin = static_cast<const BinaryOpNode*>(node);
            std::string l = lowerExpr(bin->left.get());
            std::string r = lowerExpr(bin->right.get());
            std::string tmp = newTemp();
            instrs_.push_back({tmp, bin->op, l, r});
            return tmp;
        }

        default:
            throw std::runtime_error("[IRGen] Non-expression node in lowerExpr()");
    }
}

// ---- Print helpers -----------------------------------------
void IRGenerator::printTAC() const {
    std::cout << "Three-Address Code (TAC):\n";
    for (auto& instr : instrs_) {
    if (instr.op == "if_goto")
        std::cout << "  if " << instr.arg1 << " goto " << instr.arg2 << "\n";
    else if (instr.op == "goto")
        std::cout << "  goto " << instr.arg2 << "\n";
    else if (instr.op == "label")
        std::cout << instr.result << ":\n";
    else
        std::cout << "  " << instr.toTAC() << "\n";
}
}

void IRGenerator::printLLVM() const {
    std::cout << "LLVM-style IR:\n";
    for (auto& instr : instrs_) {
    if (instr.op == "if_goto")
    std::cout << "  br i1 " << instr.arg1
              << ", label %" << instr.arg2
              << ", label %" << instr.arg2 << "\n";
    else if (instr.op == "goto")
        std::cout << "  br label %" << instr.arg2 << "\n";
    else if (instr.op == "label")
        std::cout << instr.result << ":\n";
    else
        std::cout << "  " << instr.toLLVM() << "\n";
}
}
