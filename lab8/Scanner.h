#pragma once
#include <string>
#include <vector>

enum class TokenKind { NUM, IDENT, OP, L_PAR, R_PAR, L_BRACE, R_BRACE, COMMA, SEMI, K_IF, K_ELSE, K_RET };

struct TokenUnit {
    TokenKind kind;
    std::string raw;
};

extern std::vector<TokenUnit> tokenStream;
extern int streamPos;

void runScanner(const std::string& input);
TokenUnit& activeToken();
bool isEndOfStream();
TokenUnit pullToken();