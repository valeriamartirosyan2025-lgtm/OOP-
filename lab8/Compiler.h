#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>


enum class CommandID : uint32_t {
    SET_VAL,
    GET_VAR,
    ADD_OP,
    SUB_OP,
    MUL_OP,
    DIV_OP,
    IS_EQ,
    IS_LT,
    IS_GT
};


enum class NodeType {
    Literal,
    Symbol,
    BinaryOp
};


struct RawInstruction {
    uint32_t command;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
};


struct ExpressionNode {
    NodeType kind;
    double numericData;                     
    std::string textValue;                 
    std::unique_ptr<ExpressionNode> leftSide;
    std::unique_ptr<ExpressionNode> rightSide;
};


CommandID mapOpToCommand(const std::string& op);

int generateBytecode(
    const ExpressionNode* node,
    std::vector<RawInstruction>& outCode,
    std::vector<double>& pool,
    int& regTracker,
    std::map<std::string, int>& vMap
);

#endif