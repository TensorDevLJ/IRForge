#pragma once
// ============================================================
//  PARSER  (Recursive-Descent / Predictive)
//  Phase 2 of the compiler pipeline.
//
//  Grammar implemented (EBNF):
//
//    program    → statement* EOF
//    statement  → assignment
//               | if_stmt
//
//    assignment → IDENTIFIER '=' expression ';'
//
//    if_stmt    → 'if' '(' expression ')' block
//                 ( 'else' block )?
//
//    block      → '{' statement* '}'
//
//    expression → term  ( ('+' | '-') term )*
//    term       → factor ( ('*' | '/') factor )*
//    factor     → NUMBER
//               | IDENTIFIER
//               | '(' expression ')'
//               | '-' factor            ← unary minus
//
//  Operator precedence is baked in by the call hierarchy:
//    expression (lowest)  →  term  →  factor (highest)
// ============================================================

#include "lexer.h"
#include "ast.h"
#include <vector>
#include <stdexcept>

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Entry point – returns a fully built AST
    ASTNodePtr parse();

private:
    std::vector<Token> tokens_;
    size_t             pos_;   // index of next token to consume

    // ---- Token-stream helpers --------------------------------
    const Token& peek()    const;
    const Token& advance();
    bool         check(TokenType t) const;
    bool         match(TokenType t);
    const Token& expect(TokenType t, const std::string& msg);

    // ---- Grammar rules (each returns an ASTNodePtr) ---------
    ASTNodePtr parseStatement();
    ASTNodePtr parseAssignment();
    ASTNodePtr parseIfStatement();
    std::vector<ASTNodePtr> parseBlock();
    ASTNodePtr parseExpression();
    ASTNodePtr parseTerm();
    ASTNodePtr parseFactor();
};
