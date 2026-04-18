// ============================================================
//  PARSER – implementation
// ============================================================
#include "parser.h"
#include <sstream>

// ---- constructor -------------------------------------------
Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), pos_(0) {}

// ---- token-stream helpers ----------------------------------
const Token& Parser::peek() const { return tokens_[pos_]; }

const Token& Parser::advance() {
    if (peek().type != TokenType::END_OF_FILE) ++pos_;
    return tokens_[pos_ - 1];
}

bool Parser::check(TokenType t) const {
    return peek().type == t;
}

bool Parser::match(TokenType t) {
    if (check(t)) { advance(); return true; }
    return false;
}

const Token& Parser::expect(TokenType t, const std::string& msg) {
    if (!check(t)) {
        const Token& cur = peek();
        std::ostringstream oss;
        oss << "[Parser Error] " << msg
            << "  (got " << tokenTypeName(cur.type) << "(\"" << cur.value << "\")"
            << " at line " << cur.line << ", col " << cur.col << ")";
        throw std::runtime_error(oss.str());
    }
    return advance();
}

// ---- entry point -------------------------------------------
ASTNodePtr Parser::parse() {
    auto prog = std::make_unique<ProgramNode>();
    while (!check(TokenType::END_OF_FILE))
        prog->statements.push_back(parseStatement());
    return prog;
}

// ---- statement ---------------------------------------------
ASTNodePtr Parser::parseStatement() {
    if (check(TokenType::IF))
        return parseIfStatement();
    return parseAssignment();
}

// ---- assignment: IDENTIFIER '=' expression ';' -------------
ASTNodePtr Parser::parseAssignment() {
    const Token& id = expect(TokenType::IDENTIFIER,
                             "Expected variable name at start of assignment");
    expect(TokenType::EQUAL,
           "Expected '=' after variable name '" + id.value + "'");

    ASTNodePtr expr = parseExpression();

    expect(TokenType::SEMICOLON,
           "Expected ';' at end of assignment to '" + id.value + "'");

    return std::make_unique<AssignNode>(id.value, std::move(expr));
}

// ---- if statement ------------------------------------------
ASTNodePtr Parser::parseIfStatement() {
    expect(TokenType::IF, "Expected 'if'");
    expect(TokenType::LPAREN, "Expected '(' after 'if'");
    ASTNodePtr cond = parseExpression();
    expect(TokenType::RPAREN, "Expected ')' after if-condition");

    auto thenBody = parseBlock();

    std::vector<ASTNodePtr> elseBody;
    if (match(TokenType::ELSE))
        elseBody = parseBlock();

    return std::make_unique<IfNode>(std::move(cond),
                                   std::move(thenBody),
                                   std::move(elseBody));
}

// ---- block: '{' statement* '}' ----------------------------
std::vector<ASTNodePtr> Parser::parseBlock() {
    expect(TokenType::LBRACE, "Expected '{' to open block");
    std::vector<ASTNodePtr> stmts;
    while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE))
        stmts.push_back(parseStatement());
    expect(TokenType::RBRACE, "Expected '}' to close block");
    return stmts;
}

// ---- expression: term (('+' | '-') term)* -----------------
ASTNodePtr Parser::parseExpression() {
    ASTNodePtr left = parseTerm();

    while (check(TokenType::PLUS) || check(TokenType::MINUS) ||
           check(TokenType::GT)   || check(TokenType::LT)) {
        std::string op = advance().value;   // consume the operator
        ASTNodePtr  right = parseTerm();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// ---- term: factor (('*' | '/') factor)* -------------------
ASTNodePtr Parser::parseTerm() {
    ASTNodePtr left = parseFactor();

    while (check(TokenType::MUL) || check(TokenType::DIV)) {
        std::string op = advance().value;
        ASTNodePtr  right = parseFactor();
        left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
    }
    return left;
}

// ---- factor: NUMBER | IDENTIFIER | '(' expr ')' | '-' factor
ASTNodePtr Parser::parseFactor() {
    const Token& tok = peek();

    // Integer literal
    if (tok.type == TokenType::NUMBER) {
        advance();
        return std::make_unique<NumberNode>(std::stoi(tok.value));
    }

    // Variable reference
    if (tok.type == TokenType::IDENTIFIER) {
        advance();
        return std::make_unique<IdentifierNode>(tok.value);
    }

    // Grouped expression
    if (tok.type == TokenType::LPAREN) {
        advance();   // consume '('
        ASTNodePtr expr = parseExpression();
        expect(TokenType::RPAREN, "Expected ')' to close parenthesised expression");
        return expr;
    }

    // Unary minus: treat  -x  as  0 - x
    if (tok.type == TokenType::MINUS) {
        advance();
        ASTNodePtr operand = parseFactor();
        return std::make_unique<BinaryOpNode>(
            "-",
            std::make_unique<NumberNode>(0),
            std::move(operand));
    }

    // Nothing matched
    std::ostringstream oss;
    oss << "[Parser Error] Unexpected token " << tokenTypeName(tok.type)
        << "(\"" << tok.value << "\") at line " << tok.line << ", col " << tok.col;
    throw std::runtime_error(oss.str());
}
