#pragma once
#include <string>
#include <vector>
#include <memory>

enum class NodeType {
    Literal, Symbol, BinaryOp, LogicCmp, BranchIf, Sequence, ReturnOp, AssignOp, FuncDecl, FuncInvoke
};

struct ExpressionNode {
    NodeType kind;
    std::string textValue;
    double numericData;

    std::vector<std::unique_ptr<ExpressionNode>> nested;
    std::unique_ptr<ExpressionNode> leftSide;
    std::unique_ptr<ExpressionNode> rightSide;
    std::unique_ptr<ExpressionNode> condition;
    std::vector<std::string> argList;

    ExpressionNode(NodeType k, std::string t) : kind(k), textValue(t), numericData(0) {}
    ExpressionNode(double v) : kind(NodeType::Literal), textValue(""), numericData(v) {}
};