#pragma once
#include "Token.h"
#include <string>
#include <vector>

class Lexer {
public:
    explicit Lexer(std::string source);
    Token nextToken();
    Token peekToken();
    const Token& current() const { return cur_; }

private:
    void advance();
    void skipWhitespaceAndComments();
    Token makeToken(TokenKind kind, std::string text = "");
    char peek() const;
    char get();

    std::string src_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    Token cur_;
    bool hasPeek_ = false;
    Token peek_;
};
