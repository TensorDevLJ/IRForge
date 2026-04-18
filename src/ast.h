#pragma once
// ============================================================
//  AST (Abstract Syntax Tree)
//  Phase 3 of the compiler pipeline.
//
//  Each node in the tree represents a syntactic construct:
//    - NumberNode      →  integer literal  (leaf)
//    - IdentifierNode  →  variable reference (leaf)
//    - BinaryOpNode    →  left OP right
//    - AssignNode      →  name = expr
//    - ProgramNode     →  ordered list of statements
//    - IfNode          →  if (cond) { then } [else { else_ }]
// ============================================================

#include <string>
#include <vector>
#include <memory>

// -----------------------------------------------------------
// Node type discriminator (used for printing and IR generation)
// -----------------------------------------------------------
enum class ASTNodeType {
    NUMBER,
    IDENTIFIER,
    BINARY_OP,
    ASSIGN,
    PROGRAM,
    IF_STMT
};

// -----------------------------------------------------------
// Base AST node – all concrete nodes inherit from this.
// Using unique_ptr for ownership (no manual delete needed).
// -----------------------------------------------------------
struct ASTNode {
    ASTNodeType type;
    virtual ~ASTNode() = default;

    // Pretty-print the subtree.  indent grows by 2 spaces per level.
    virtual void print(int indent = 0) const = 0;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

// -----------------------------------------------------------
// Leaf: integer literal
// -----------------------------------------------------------
struct NumberNode : public ASTNode {
    int value;
    explicit NumberNode(int v);
    void print(int indent = 0) const override;
};

// -----------------------------------------------------------
// Leaf: variable name reference
// -----------------------------------------------------------
struct IdentifierNode : public ASTNode {
    std::string name;
    explicit IdentifierNode(std::string n);
    void print(int indent = 0) const override;
};

// -----------------------------------------------------------
// Interior: binary arithmetic or comparison operation
// -----------------------------------------------------------
struct BinaryOpNode : public ASTNode {
    std::string op;    // "+", "-", "*", "/", ">", "<"
    ASTNodePtr  left;
    ASTNodePtr  right;

    BinaryOpNode(std::string op, ASTNodePtr l, ASTNodePtr r);
    void print(int indent = 0) const override;
};

// -----------------------------------------------------------
// Statement: variable assignment  (name = expression)
// -----------------------------------------------------------
struct AssignNode : public ASTNode {
    std::string name;       // left-hand side variable
    ASTNodePtr  expr;       // right-hand side expression

    AssignNode(std::string name, ASTNodePtr expr);
    void print(int indent = 0) const override;
};

// -----------------------------------------------------------
// Optional: if statement
// -----------------------------------------------------------
struct IfNode : public ASTNode {
    ASTNodePtr              condition;
    std::vector<ASTNodePtr> thenBody;
    std::vector<ASTNodePtr> elseBody;   // empty if no else branch

    IfNode(ASTNodePtr cond,
           std::vector<ASTNodePtr> then_,
           std::vector<ASTNodePtr> else_);
    void print(int indent = 0) const override;
};

// -----------------------------------------------------------
// Root: ordered list of statements
// -----------------------------------------------------------
struct ProgramNode : public ASTNode {
    std::vector<ASTNodePtr> statements;
    ProgramNode();
    void print(int indent = 0) const override;
};
