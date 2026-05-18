#pragma once
#include "AST.h"
#include "ExecFileStructure.h"
#include <map>
#include <string>

struct CompileResult {
    ExecFile exec;
    std::map<std::string, int> functionTable;
};

CompileResult compileProgram(const ExpressionNode* program);
