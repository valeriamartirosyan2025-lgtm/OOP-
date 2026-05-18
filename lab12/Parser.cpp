#include "Parser.h"

Parser::Parser(Lexer& lex) : lex_(lex) {
    cur_ = lex_.nextToken();
}

bool Parser::check(TokenKind k) const { return cur_.kind == k; }
bool Parser::match(TokenKind k) {
    if (!check(k)) return false;
    cur_ = lex_.nextToken();
    return true;
}

Token Parser::consume(TokenKind expected) {
    if (!check(expected)) {
        throw ParseError("expected token, got '" + cur_.text + "'",
                         cur_.line, cur_.column);
    }
    Token t = cur_;
    cur_ = lex_.nextToken();
    return t;
}

CmpOp Parser::tokenToCmp(TokenKind k) {
    switch (k) {
    case TokenKind::Eq: return CmpOp::EQ;
    case TokenKind::Ne: return CmpOp::NE;
    case TokenKind::Lt: return CmpOp::LT;
    case TokenKind::Le: return CmpOp::LE;
    case TokenKind::Gt: return CmpOp::GT;
    case TokenKind::Ge: return CmpOp::GE;
    default: return CmpOp::EQ;
    }
}

std::unique_ptr<ExpressionNode> Parser::parseProgram() {
    auto program = std::make_unique<ExpressionNode>(NodeType::Program, "");
    while (!check(TokenKind::End))
        program->nested.push_back(parseTopLevel());
    return program;
}

std::unique_ptr<ExpressionNode> Parser::parseTopLevel() {
    bool isStatic = match(TokenKind::StaticKw);
    consume(TokenKind::IntKw);
    std::string name = consume(TokenKind::Identifier).text;

    if (match(TokenKind::LParen)) {
        auto fn = std::make_unique<ExpressionNode>(NodeType::FuncDecl, name);
        if (!check(TokenKind::RParen)) {
            do {
                match(TokenKind::IntKw);
                fn->argList.push_back(consume(TokenKind::Identifier).text);
            } while (match(TokenKind::Comma));
        }
        consume(TokenKind::RParen);
        fn->body = parseBlock();
        return fn;
    }

    auto var = std::make_unique<ExpressionNode>(NodeType::VarDecl, name);
    var->storage = isStatic ? VarStorage::Static : VarStorage::Global;
    if (match(TokenKind::Assign))
        var->rightSide = parseExpression();
    consume(TokenKind::Semicolon);
    return var;
}

std::unique_ptr<ExpressionNode> Parser::parseBlock() {
    consume(TokenKind::LBrace);
    auto block = std::make_unique<ExpressionNode>(NodeType::Block, "");
    while (!check(TokenKind::RBrace) && !check(TokenKind::End))
        block->nested.push_back(parseStatement());
    consume(TokenKind::RBrace);
    return block;
}

std::unique_ptr<ExpressionNode> Parser::parseStatement() {
    if (match(TokenKind::IntKw)) {
        auto decl = parseVarDecl(VarStorage::Local);
        consume(TokenKind::Semicolon);
        return decl;
    }
    if (match(TokenKind::IfKw)) return parseIf();
    if (match(TokenKind::WhileKw)) return parseWhile();
    if (match(TokenKind::DoKw)) return parseDoWhile();
    if (match(TokenKind::ReturnKw)) return parseReturn();
    if (check(TokenKind::LBrace)) return parseBlock();

    auto expr = parseAssign();
    consume(TokenKind::Semicolon);
    auto stmt = std::make_unique<ExpressionNode>(NodeType::ExprStmt, "");
    stmt->leftSide = std::move(expr);
    return stmt;
}

std::unique_ptr<ExpressionNode> Parser::parseVarDecl(VarStorage storage) {
    std::string name = consume(TokenKind::Identifier).text;
    auto node = std::make_unique<ExpressionNode>(NodeType::VarDecl, name);
    node->storage = storage;
    if (match(TokenKind::Assign))
        node->rightSide = parseExpression();
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseIf() {
    consume(TokenKind::LParen);
    auto node = std::make_unique<ExpressionNode>(NodeType::If, "");
    node->condition = parseExpression();
    consume(TokenKind::RParen);
    node->thenBranch = parseStatement();
    if (match(TokenKind::ElseKw))
        node->elseBranch = parseStatement();
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseWhile() {
    auto node = std::make_unique<ExpressionNode>(NodeType::While, "");
    consume(TokenKind::LParen);
    node->condition = parseExpression();
    consume(TokenKind::RParen);
    node->body = parseStatement();
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseDoWhile() {
    auto node = std::make_unique<ExpressionNode>(NodeType::DoWhile, "");
    node->body = parseStatement();
    consume(TokenKind::WhileKw);
    consume(TokenKind::LParen);
    node->condition = parseExpression();
    consume(TokenKind::RParen);
    consume(TokenKind::Semicolon);
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseReturn() {
    auto node = std::make_unique<ExpressionNode>(NodeType::Return, "");
    if (!check(TokenKind::Semicolon))
        node->leftSide = parseExpression();
    consume(TokenKind::Semicolon);
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseExpression() {
    return parseCompare();
}

std::unique_ptr<ExpressionNode> Parser::parseAssign() {
    auto left = parseCompare();
    if (match(TokenKind::Assign)) {
        if (left->kind != NodeType::Symbol)
            throw ParseError("invalid assignment target", cur_.line, cur_.column);
        auto node = std::make_unique<ExpressionNode>(NodeType::Assign, left->textValue);
        node->leftSide = std::move(left);
        node->rightSide = parseAssign();
        return node;
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseCompare() {
    auto left = parseTerm();
    if (check(TokenKind::Eq) || check(TokenKind::Ne) || check(TokenKind::Lt) ||
        check(TokenKind::Le) || check(TokenKind::Gt) || check(TokenKind::Ge)) {
        TokenKind op = cur_.kind;
        cur_ = lex_.nextToken();
        auto node = std::make_unique<ExpressionNode>(NodeType::LogicCmp, "");
        node->cmpOp = tokenToCmp(op);
        node->leftSide = std::move(left);
        node->rightSide = parseTerm();
        return node;
    }
    return left;
}

std::unique_ptr<ExpressionNode> Parser::parseTerm() {
    auto node = parseFactor();
    while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
        std::string op = cur_.text;
        cur_ = lex_.nextToken();
        auto bin = std::make_unique<ExpressionNode>(NodeType::BinaryOp, op);
        bin->leftSide = std::move(node);
        bin->rightSide = parseFactor();
        node = std::move(bin);
    }
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parseFactor() {
    auto node = parsePrimary();
    while (check(TokenKind::Star) || check(TokenKind::Slash)) {
        std::string op = cur_.text;
        cur_ = lex_.nextToken();
        auto bin = std::make_unique<ExpressionNode>(NodeType::BinaryOp, op);
        bin->leftSide = std::move(node);
        bin->rightSide = parsePrimary();
        node = std::move(bin);
    }
    return node;
}

std::unique_ptr<ExpressionNode> Parser::parsePrimary() {
    if (check(TokenKind::Number)) {
        double v = std::stod(consume(TokenKind::Number).text);
        return std::make_unique<ExpressionNode>(v);
    }
    if (check(TokenKind::Identifier)) {
        std::string name = consume(TokenKind::Identifier).text;
        if (match(TokenKind::LParen)) {
            auto call = std::make_unique<ExpressionNode>(NodeType::FuncInvoke, name);
            if (!check(TokenKind::RParen)) {
                do {
                    call->nested.push_back(parseExpression());
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RParen);
            return call;
        }
        return std::make_unique<ExpressionNode>(NodeType::Symbol, name);
    }
    if (match(TokenKind::LParen)) {
        auto e = parseExpression();
        consume(TokenKind::RParen);
        return e;
    }
    if (match(TokenKind::Minus)) {
        auto u = std::make_unique<ExpressionNode>(NodeType::UnaryOp, "-");
        u->leftSide = parsePrimary();
        return u;
    }
    throw ParseError("expected expression", cur_.line, cur_.column);
}
