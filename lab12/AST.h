#pragma once
#include <memory>
#include <string>
#include <vector>

enum class VarStorage { Local, Global, Static };

enum class NodeType {
    Program,
    FuncDecl,
    Block,
    VarDecl,
    Assign,
    Return,
    If,
    While,
    DoWhile,
    ExprStmt,
    Literal,
    Symbol,
    BinaryOp,
    UnaryOp,
    LogicCmp,
    FuncInvoke
};

enum class CmpOp { EQ, NE, LT, LE, GT, GE };

struct ExpressionNode {
    NodeType kind = NodeType::Literal;
    std::string textValue;
    double numericData = 0;
    VarStorage storage = VarStorage::Local;
    CmpOp cmpOp = CmpOp::EQ;

    std::vector<std::unique_ptr<ExpressionNode>> nested;
    std::unique_ptr<ExpressionNode> leftSide;
    std::unique_ptr<ExpressionNode> rightSide;
    std::unique_ptr<ExpressionNode> condition;
    std::unique_ptr<ExpressionNode> thenBranch;
    std::unique_ptr<ExpressionNode> elseBranch;
    std::unique_ptr<ExpressionNode> body;
    std::vector<std::string> argList;

    ExpressionNode() = default;
    ExpressionNode(NodeType k, std::string t) : kind(k), textValue(std::move(t)) {}
    explicit ExpressionNode(double v) : kind(NodeType::Literal), numericData(v) {}
};
