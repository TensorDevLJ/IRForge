// ============================================================
//  AST – implementation
// ============================================================
#include "ast.h"
#include <iostream>

// Helper: produce an indentation string
static std::string indent_str(int n) { return std::string(n * 2, ' '); }

// -----------------------------------------------------------
// NumberNode
// -----------------------------------------------------------
NumberNode::NumberNode(int v) {
    type  = ASTNodeType::NUMBER;
    value = v;
}
void NumberNode::print(int indent) const {
    std::cout << indent_str(indent) << "NUMBER(" << value << ")\n";
}

// -----------------------------------------------------------
// IdentifierNode
// -----------------------------------------------------------
IdentifierNode::IdentifierNode(std::string n) {
    type = ASTNodeType::IDENTIFIER;
    name = std::move(n);
}
void IdentifierNode::print(int indent) const {
    std::cout << indent_str(indent) << "IDENTIFIER(" << name << ")\n";
}

// -----------------------------------------------------------
// BinaryOpNode
// -----------------------------------------------------------
BinaryOpNode::BinaryOpNode(std::string o, ASTNodePtr l, ASTNodePtr r) {
    type  = ASTNodeType::BINARY_OP;
    op    = std::move(o);
    left  = std::move(l);
    right = std::move(r);
}
void BinaryOpNode::print(int indent) const {
    std::cout << indent_str(indent) << "BINARY_OP(" << op << ")\n";
    left->print(indent + 1);
    right->print(indent + 1);
}

// -----------------------------------------------------------
// AssignNode
// -----------------------------------------------------------
AssignNode::AssignNode(std::string n, ASTNodePtr e) {
    type = ASTNodeType::ASSIGN;
    name = std::move(n);
    expr = std::move(e);
}
void AssignNode::print(int indent) const {
    std::cout << indent_str(indent) << "ASSIGN(" << name << ")\n";
    expr->print(indent + 1);
}

// -----------------------------------------------------------
// IfNode
// -----------------------------------------------------------
IfNode::IfNode(ASTNodePtr cond,
               std::vector<ASTNodePtr> then_,
               std::vector<ASTNodePtr> else_)
{
    type      = ASTNodeType::IF_STMT;
    condition = std::move(cond);
    thenBody  = std::move(then_);
    elseBody  = std::move(else_);
}
void IfNode::print(int indent) const {
    std::cout << indent_str(indent) << "IF\n";
    std::cout << indent_str(indent + 1) << "CONDITION:\n";
    condition->print(indent + 2);
    std::cout << indent_str(indent + 1) << "THEN:\n";
    for (auto& s : thenBody)  s->print(indent + 2);
    if (!elseBody.empty()) {
        std::cout << indent_str(indent + 1) << "ELSE:\n";
        for (auto& s : elseBody)  s->print(indent + 2);
    }
}

// -----------------------------------------------------------
// ProgramNode
// -----------------------------------------------------------
ProgramNode::ProgramNode() { type = ASTNodeType::PROGRAM; }
void ProgramNode::print(int indent) const {
    std::cout << indent_str(indent) << "PROGRAM\n";
    for (auto& stmt : statements)
        stmt->print(indent + 1);
}
