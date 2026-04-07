#include "SyntaxEngine.h"
#include "Scanner.h"

namespace GrammarEngine {
    static std::unique_ptr<ExpressionNode> parseLeaf() {
        if (isEndOfStream()) return nullptr;

        TokenUnit t = activeToken();
        if (t.kind == TokenKind::NUM) {
            return std::make_unique<ExpressionNode>(std::stod(pullToken().raw));
        }
        if (t.kind == TokenKind::IDENT) {
            return std::make_unique<ExpressionNode>(NodeType::Symbol, pullToken().raw);
        }
        if (t.kind == TokenKind::L_PAR) {
            pullToken(); 
            auto node = parseAddition();
            if (!isEndOfStream() && activeToken().kind == TokenKind::R_PAR) pullToken(); 
            return node;
        }
        return nullptr;
    }

    
    static std::unique_ptr<ExpressionNode> parseMultiplication() {
        auto node = parseLeaf();
        while (!isEndOfStream() && (activeToken().raw == "*" || activeToken().raw == "/")) {
            std::string op = pullToken().raw;
            auto parent = std::make_unique<ExpressionNode>(NodeType::BinaryOp, op);
            parent->leftSide = std::move(node);
            parent->rightSide = parseLeaf();
            node = std::move(parent);
        }
        return node;
    }

 
    static std::unique_ptr<ExpressionNode> parseAddition() {
        auto node = parseMultiplication();
        while (!isEndOfStream() && (activeToken().raw == "+" || activeToken().raw == "-")) {
            std::string op = pullToken().raw;
            auto parent = std::make_unique<ExpressionNode>(NodeType::BinaryOp, op);
            parent->leftSide = std::move(node);
            parent->rightSide = parseMultiplication();
            node = std::move(parent);
        }
        return node;
    }
}

std::unique_ptr<ExpressionNode> constructAST() {
    return GrammarEngine::parseAddition();
}