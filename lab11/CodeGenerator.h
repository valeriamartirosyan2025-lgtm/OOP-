#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "AST.h"
#include "ExecFileStructure.h"
#include <map>
#include <string>

CommandID mapOpToCommand(const std::string& op);

int generateBytecode(
    const ExpressionNode* node,
    ExecFile& exec,
    int& regTracker,
    std::map<std::string, int>& vMap
);

#endif