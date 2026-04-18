// ============================================================
//  LEXER – implementation
// ============================================================
#include "lexer.h"
#include <cctype>
#include <sstream>
#include <unordered_map>

// Reserved keywords  (identifier text  →  token type)
static const std::unordered_map<std::string, TokenType> KEYWORDS = {
    {"if",   TokenType::IF},
    {"else", TokenType::ELSE}
};

// ---- constructor -------------------------------------------
Lexer::Lexer(const std::string& source)
    : src_(source), pos_(0), line_(1), col_(1) {}

// ---- helpers -----------------------------------------------
bool Lexer::isAtEnd() const { return pos_ >= src_.size(); }

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return src_[pos_];
}

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') { ++line_; col_ = 1; }
    else           { ++col_; }
    return c;
}

Token Lexer::makeToken(TokenType t, const std::string& v) const {
    // col_ has already advanced past the token; adjust back
    return Token{t, v, line_, col_};
}

void Lexer::skipWhitespace() {
    while (!isAtEnd() && std::isspace(static_cast<unsigned char>(peek())))
        advance();
}

// ---- read an integer literal --------------------------------
Token Lexer::readNumber() {
    int startLine = line_, startCol = col_;
    std::string num;
    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek())))
        num += advance();
    return Token{TokenType::NUMBER, num, startLine, startCol};
}

// ---- read an identifier or keyword --------------------------
Token Lexer::readIdentifierOrKeyword() {
    int startLine = line_, startCol = col_;
    std::string ident;
    while (!isAtEnd() &&
           (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_'))
        ident += advance();

    // Check if it is a reserved word
    auto it = KEYWORDS.find(ident);
    TokenType t = (it != KEYWORDS.end()) ? it->second : TokenType::IDENTIFIER;
    return Token{t, ident, startLine, startCol};
}

// ---- main tokenise pass -------------------------------------
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespace();
        if (isAtEnd()) {
            tokens.push_back(Token{TokenType::END_OF_FILE, "", line_, col_});
            break;
        }

        int startLine = line_, startCol = col_;
        char c = peek();

        // --- numeric literal
        if (std::isdigit(static_cast<unsigned char>(c))) {
            tokens.push_back(readNumber());
            continue;
        }

        // --- identifier / keyword
        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            tokens.push_back(readIdentifierOrKeyword());
            continue;
        }

        // --- single-character operators & punctuation
        advance();   // consume the character
        switch (c) {
            case '+': tokens.push_back(Token{TokenType::PLUS,      "+", startLine, startCol}); break;
            case '-': tokens.push_back(Token{TokenType::MINUS,     "-", startLine, startCol}); break;
            case '*': tokens.push_back(Token{TokenType::MUL,       "*", startLine, startCol}); break;
            case '/': tokens.push_back(Token{TokenType::DIV,       "/", startLine, startCol}); break;
            case '=': tokens.push_back(Token{TokenType::EQUAL,     "=", startLine, startCol}); break;
            case '(': tokens.push_back(Token{TokenType::LPAREN,    "(", startLine, startCol}); break;
            case ')': tokens.push_back(Token{TokenType::RPAREN,    ")", startLine, startCol}); break;
            case ';': tokens.push_back(Token{TokenType::SEMICOLON, ";", startLine, startCol}); break;
            case '>': tokens.push_back(Token{TokenType::GT,        ">", startLine, startCol}); break;
            case '<': tokens.push_back(Token{TokenType::LT,        "<", startLine, startCol}); break;
            case '{': tokens.push_back(Token{TokenType::LBRACE,    "{", startLine, startCol}); break;
            case '}': tokens.push_back(Token{TokenType::RBRACE,    "}", startLine, startCol}); break;
            default: {
                std::ostringstream oss;
                oss << "[Lexer Error] Unexpected character '" << c
                    << "' at line " << startLine << ", col " << startCol;
                throw std::runtime_error(oss.str());
            }
        }
    }

    return tokens;
}
