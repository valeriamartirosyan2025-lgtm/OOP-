#include "Compiler.h"
#include <stdexcept>

CommandID mapOpToCommand(const std::string& op) {
    if (op == "+") return CommandID::ADD_OP;
    if (op == "-") return CommandID::SUB_OP;
    if (op == "*") return CommandID::MUL_OP;
    if (op == "/") return CommandID::DIV_OP;
    if (op == "==") return CommandID::IS_EQ;
    if (op == "<") return CommandID::IS_LT;
    if (op == ">") return CommandID::IS_GT;
    throw std::runtime_error("Անհայտ գործողություն: " + op);
}

int generateBytecode(const ExpressionNode* node, std::vector<RawInstruction>& outCode, std::vector<double>& pool, int& regTracker, std::map<std::string, int>& vMap) {
    if (!node) return -1;

    switch (node->kind) {
    case NodeType::Literal: {
        int targetReg = regTracker++;
        uint32_t constIdx = (uint32_t)pool.size();
        pool.push_back(node->numericData);
      
        outCode.push_back({ (uint32_t)CommandID::SET_VAL, (uint32_t)targetReg, constIdx, 0 });
        return targetReg;
    }

    case NodeType::Symbol: {
        int targetReg = regTracker++;
        
        if (vMap.find(node->textValue) == vMap.end()) {
            vMap[node->textValue] = (int)vMap.size();
        }
        int varIdx = vMap[node->textValue];
        outCode.push_back({ (uint32_t)CommandID::GET_VAR, (uint32_t)targetReg, (uint32_t)varIdx, 0 });
        return targetReg;
    }

    case NodeType::BinaryOp: {
        
        int leftReg = generateBytecode(node->leftSide.get(), outCode, pool, regTracker, vMap);
        int rightReg = generateBytecode(node->rightSide.get(), outCode, pool, regTracker, vMap);

        int resultReg = regTracker++;
        CommandID cmd = mapOpToCommand(node->textValue);

        
        outCode.push_back({ (uint32_t)cmd, (uint32_t)resultReg, (uint32_t)leftReg, (uint32_t)rightReg });
        return resultReg;
    }

    default:
        return -1;
    }
}