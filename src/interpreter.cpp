// ============================================================
//  INTERPRETER – implementation
// ============================================================
#include "interpreter.h"
#include <stdexcept>
#include <iostream>

Interpreter::Interpreter(SymbolTable& symTable) : sym_(symTable) {}

// ---- public entry ------------------------------------------
void Interpreter::run(const ASTNode* node) {
    if (!node) return;

    if (node->type == ASTNodeType::PROGRAM) {
        const auto* prog = static_cast<const ProgramNode*>(node);
        for (auto& stmt : prog->statements)
            run(stmt.get());
        return;
    }

    if (node->type == ASTNodeType::ASSIGN) {
        const auto* asgn = static_cast<const AssignNode*>(node);
        int val = eval(asgn->expr.get());
        sym_.set(asgn->name, val);
        std::cout << "  >> " << asgn->name << " = " << val << "\n";
        return;
    }

    if (node->type == ASTNodeType::IF_STMT) {
        const auto* ifn = static_cast<const IfNode*>(node);
        int condVal = eval(ifn->condition.get());
        if (condVal) {
            for (auto& s : ifn->thenBody) run(s.get());
        } else {
            for (auto& s : ifn->elseBody) run(s.get());
        }
        return;
    }

    throw std::runtime_error("[Interpreter] Unknown statement node type");
}

// ---- evaluate expression -----------------------------------
int Interpreter::eval(const ASTNode* node) {
    switch (node->type) {
        case ASTNodeType::NUMBER:
            return static_cast<const NumberNode*>(node)->value;

        case ASTNodeType::IDENTIFIER: {
            const auto* id = static_cast<const IdentifierNode*>(node);
            return sym_.get(id->name);  // throws if undefined
        }

        case ASTNodeType::BINARY_OP: {
            const auto* bin = static_cast<const BinaryOpNode*>(node);
            int lv = eval(bin->left.get());
            int rv = eval(bin->right.get());
            if (bin->op == "+") return lv + rv;
            if (bin->op == "-") return lv - rv;
            if (bin->op == "*") return lv * rv;
            if (bin->op == "/") {
                if (rv == 0)
                    throw std::runtime_error("[Runtime Error] Division by zero");
                return lv / rv;
            }
            if (bin->op == ">") return lv > rv ? 1 : 0;
            if (bin->op == "<") return lv < rv ? 1 : 0;
            throw std::runtime_error("[Runtime Error] Unknown operator: " + bin->op);
        }

        default:
            throw std::runtime_error("[Interpreter] Non-expression node in eval()");
    }
}
