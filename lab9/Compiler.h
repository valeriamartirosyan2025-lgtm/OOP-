#ifndef COMPILER_H
#define COMPILER_H

#include "NodeStructure.h"
#include "BytecodeDefs.h"
#include <string>
#include <vector>
#include <map>
#include <memory>

CommandID mapOpToCommand(const std::string& op);

int generateBytecode(
    const ExpressionNode* node,
    std::vector<RawInstruction>& outCode,
    std::vector<double>& pool,
    int& regTracker,
    std::map<std::string, int>& vMap
);

#endif