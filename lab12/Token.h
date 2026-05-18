#pragma once
#include <string>

enum class TokenKind {
    End,
    IntKw, IfKw, ElseKw, WhileKw, DoKw, ReturnKw, StaticKw,
    Identifier, Number,
    Plus, Minus, Star, Slash,
    Assign, Eq, Ne, Lt, Le, Gt, Ge,
    LParen, RParen, LBrace, RBrace, Comma, Semicolon,
};

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;
    int line = 1;
    int column = 1;
};
