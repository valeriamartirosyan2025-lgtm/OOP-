#pragma once
#include "AST.h"
#include "Lexer.h"
#include <memory>
#include <stdexcept>

class ParseError : public std::runtime_error {
public:
    ParseError(const std::string& msg, int line, int col)
        : std::runtime_error("Parse error at " + std::to_string(line) + ":" +
                             std::to_string(col) + ": " + msg) {}
};

class Parser {
public:
    explicit Parser(Lexer& lex);
    std::unique_ptr<ExpressionNode> parseProgram();

private:
    Token consume(TokenKind expected);
    bool match(TokenKind k);
    bool check(TokenKind k) const;

    std::unique_ptr<ExpressionNode> parseTopLevel();
    std::unique_ptr<ExpressionNode> parseBlock();
    std::unique_ptr<ExpressionNode> parseStatement();
    std::unique_ptr<ExpressionNode> parseVarDecl(VarStorage storage);
    std::unique_ptr<ExpressionNode> parseIf();
    std::unique_ptr<ExpressionNode> parseWhile();
    std::unique_ptr<ExpressionNode> parseDoWhile();
    std::unique_ptr<ExpressionNode> parseReturn();
    std::unique_ptr<ExpressionNode> parseExpression();
    std::unique_ptr<ExpressionNode> parseAssign();
    std::unique_ptr<ExpressionNode> parseCompare();
    std::unique_ptr<ExpressionNode> parseTerm();
    std::unique_ptr<ExpressionNode> parseFactor();
    std::unique_ptr<ExpressionNode> parsePrimary();
    CmpOp tokenToCmp(TokenKind k);

    Lexer& lex_;
    Token cur_;
};
