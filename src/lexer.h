#pragma once
// ============================================================
//  LEXER (Tokenizer / Lexical Analyser)
//  Phase 1 of the compiler pipeline.
//
//  Responsibility:
//    - Scan the raw source string character by character
//    - Classify runs of characters into typed tokens
//    - Discard whitespace / comments
//    - Report unrecognised characters with a helpful error
// ============================================================

#include <string>
#include <vector>
#include <stdexcept>

// -----------------------------------------------------------
// Token types supported by our simple C-like language
// -----------------------------------------------------------
enum class TokenType {
    NUMBER,       // integer literal  e.g. 42
    IDENTIFIER,   // variable name    e.g. myVar
    PLUS,         // +
    MINUS,        // -
    MUL,          // *
    DIV,          // /
    EQUAL,        // =   (assignment)
    LPAREN,       // (
    RPAREN,       // )
    SEMICOLON,    // ;
    GT,           // >   (greater-than, for optional extensions)
    LT,           // <   (less-than)
    IF,           // keyword "if"
    ELSE,         // keyword "else"
    LBRACE,       // {
    RBRACE,       // }
    END_OF_FILE   // sentinel – no more input
};

// -----------------------------------------------------------
// Human-readable name for a token type (used in error msgs
// and debug output)
// -----------------------------------------------------------
inline std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:      return "NUMBER";
        case TokenType::IDENTIFIER:  return "IDENTIFIER";
        case TokenType::PLUS:        return "PLUS";
        case TokenType::MINUS:       return "MINUS";
        case TokenType::MUL:         return "MUL";
        case TokenType::DIV:         return "DIV";
        case TokenType::EQUAL:       return "EQUAL";
        case TokenType::LPAREN:      return "LPAREN";
        case TokenType::RPAREN:      return "RPAREN";
        case TokenType::SEMICOLON:   return "SEMICOLON";
        case TokenType::GT:          return "GT";
        case TokenType::LT:          return "LT";
        case TokenType::IF:          return "IF";
        case TokenType::ELSE:        return "ELSE";
        case TokenType::LBRACE:      return "LBRACE";
        case TokenType::RBRACE:      return "RBRACE";
        case TokenType::END_OF_FILE: return "EOF";
        default:                     return "UNKNOWN";
    }
}

// -----------------------------------------------------------
// A single lexical token
// -----------------------------------------------------------
struct Token {
    TokenType   type;
    std::string value;   // raw text matched from source
    int         line;    // 1-based source line (for error messages)
    int         col;     // 1-based column

    std::string toString() const {
        return tokenTypeName(type) + "(\"" + value + "\") @" +
               std::to_string(line) + ":" + std::to_string(col);
    }
};

// -----------------------------------------------------------
// Lexer class
// -----------------------------------------------------------
class Lexer {
public:
    explicit Lexer(const std::string& source);

    // Tokenise the whole input and return the token list.
    // Throws std::runtime_error on unrecognised characters.
    std::vector<Token> tokenize();

private:
    std::string src_;   // raw source text
    size_t      pos_;   // current read position
    int         line_;  // current line (1-based)
    int         col_;   // current column (1-based)

    char  peek()   const;           // look at current char without advancing
    char  advance();                // consume and return current char
    bool  isAtEnd() const;

    void  skipWhitespace();
    Token readNumber();
    Token readIdentifierOrKeyword();
    Token makeToken(TokenType t, const std::string& v) const;
};
