#include "CodeGenerator.h"
#include <stdexcept>

CommandID mapOpToCommand(const std::string& op) {
    if (op == "+") return CommandID::ADD;
    if (op == "-") return CommandID::SUB;
    if (op == "*") return CommandID::MUL;
    if (op == "/") return CommandID::DIV;
    if (op == "==") return CommandID::BR_EQ;
    if (op == "<") return CommandID::BR_LT;
    if (op == ">") return CommandID::BR_GT;
    throw std::runtime_error("Անհայտ գործողություն: " + op);
}

int generateBytecode(const ExpressionNode* node, ExecFile& exec, int& regTracker, std::map<std::string, int>& vMap) {
    if (!node) return 0;

    switch (node->kind) {
    case NodeType::Literal: {
        int targetReg = regTracker++;
        uint32_t dataIdx = (uint32_t)exec.dataSegment.size();
        exec.dataSegment.push_back((int32_t)node->numericData);

        exec.codeSegment.push_back({ (uint32_t)CommandID::SET_VAL, (uint32_t)targetReg, 0, (int32_t)dataIdx });
        return targetReg;
    }

    case NodeType::Symbol: {
        int targetReg = regTracker++;
        if (vMap.find(node->textValue) == vMap.end()) {
            vMap[node->textValue] = (int)vMap.size();
            exec.dataSegment.push_back(0);
        }
        int varIdx = vMap[node->textValue];
        exec.codeSegment.push_back({ (uint32_t)CommandID::GET_VAR, (uint32_t)targetReg, 0, varIdx });
        return targetReg;
    }

    case NodeType::BinaryOp: {
        int leftReg = generateBytecode(node->leftSide.get(), exec, regTracker, vMap);
        int rightReg = generateBytecode(node->rightSide.get(), exec, regTracker, vMap);

        CommandID cmd = mapOpToCommand(node->textValue);
        int resultReg = regTracker++;

        if (cmd == CommandID::BR_EQ || cmd == CommandID::BR_LT || cmd == CommandID::BR_GT) {
            
            exec.codeSegment.push_back({ (uint32_t)cmd, (uint32_t)leftReg, (uint32_t)rightReg, 8 });
           
            exec.codeSegment.push_back({ (uint32_t)CommandID::SET_VAL, (uint32_t)resultReg, 0, 0 });
           
            exec.codeSegment.push_back({ (uint32_t)CommandID::BR_EQ, 0, 0, 4 });
         
            exec.codeSegment.push_back({ (uint32_t)CommandID::SET_VAL, (uint32_t)resultReg, 0, 1 });
        }
        else {
            exec.codeSegment.push_back({ (uint32_t)cmd, (uint32_t)leftReg, (uint32_t)rightReg, (uint32_t)resultReg });
        }
        return resultReg;
    }
    default:
        return 0;
    }
}