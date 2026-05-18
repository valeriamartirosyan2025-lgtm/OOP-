#include "Lexer.h"
#include <cctype>
#include <unordered_map>

Lexer::Lexer(std::string source) : src_(std::move(source)) {
    advance();
}

char Lexer::peek() const {
    return pos_ < src_.size() ? src_[pos_] : '\0';
}

char Lexer::get() {
    if (pos_ >= src_.size()) return '\0';
    char c = src_[pos_++];
    if (c == '\n') { line_++; col_ = 1; }
    else col_++;
    return c;
}

void Lexer::advance() {
    if (hasPeek_) {
        cur_ = peek_;
        hasPeek_ = false;
        return;
    }
    skipWhitespaceAndComments();
    int startLine = line_;
    int startCol = col_;

    char c = peek();
    if (c == '\0') {
        cur_ = { TokenKind::End, "", startLine, startCol };
        return;
    }

    if (std::isalpha((unsigned char)c) || c == '_') {
        std::string id;
        while (std::isalnum((unsigned char)peek()) || peek() == '_')
            id += get();
        static const std::unordered_map<std::string, TokenKind> kw = {
            {"int", TokenKind::IntKw}, {"if", TokenKind::IfKw},
            {"else", TokenKind::ElseKw}, {"while", TokenKind::WhileKw},
            {"do", TokenKind::DoKw}, {"return", TokenKind::ReturnKw},
            {"static", TokenKind::StaticKw},
        };
        auto it = kw.find(id);
        cur_ = { it != kw.end() ? it->second : TokenKind::Identifier, id, startLine, startCol };
        return;
    }

    if (std::isdigit((unsigned char)c)) {
        std::string num;
        while (std::isdigit((unsigned char)peek())) num += get();
        cur_ = { TokenKind::Number, num, startLine, startCol };
        return;
    }

    get();
    switch (c) {
    case '+': cur_ = makeToken(TokenKind::Plus, "+"); break;
    case '-': cur_ = makeToken(TokenKind::Minus, "-"); break;
    case '*': cur_ = makeToken(TokenKind::Star, "*"); break;
    case '/': cur_ = makeToken(TokenKind::Slash, "/"); break;
    case '(': cur_ = makeToken(TokenKind::LParen); break;
    case ')': cur_ = makeToken(TokenKind::RParen); break;
    case '{': cur_ = makeToken(TokenKind::LBrace); break;
    case '}': cur_ = makeToken(TokenKind::RBrace); break;
    case ',': cur_ = makeToken(TokenKind::Comma); break;
    case ';': cur_ = makeToken(TokenKind::Semicolon); break;
    case '=':
        if (peek() == '=') { get(); cur_ = makeToken(TokenKind::Eq, "=="); }
        else cur_ = makeToken(TokenKind::Assign, "=");
        break;
    case '!':
        if (peek() == '=') { get(); cur_ = makeToken(TokenKind::Ne, "!="); }
        else cur_ = makeToken(TokenKind::End, "!");
        break;
    case '<':
        if (peek() == '=') { get(); cur_ = makeToken(TokenKind::Le, "<="); }
        else cur_ = makeToken(TokenKind::Lt, "<");
        break;
    case '>':
        if (peek() == '=') { get(); cur_ = makeToken(TokenKind::Ge, ">="); }
        else cur_ = makeToken(TokenKind::Gt, ">");
        break;
    default:
        cur_ = makeToken(TokenKind::End, std::string(1, c));
        break;
    }
    cur_.line = startLine;
    cur_.column = startCol;
}

Token Lexer::makeToken(TokenKind kind, std::string text) {
    return { kind, std::move(text), line_, col_ };
}

void Lexer::skipWhitespaceAndComments() {
    for (;;) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            get();
            continue;
        }
        if (c == '/' && pos_ + 1 < src_.size() && src_[pos_ + 1] == '/') {
            while (peek() && peek() != '\n') get();
            continue;
        }
        break;
    }
}

Token Lexer::nextToken() {
    Token t = cur_;
    advance();
    return t;
}

Token Lexer::peekToken() {
    if (!hasPeek_) {
        peek_ = cur_;
        advance();
        hasPeek_ = true;
    }
    return peek_;
}
