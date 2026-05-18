#pragma once
#include "ExecFileStructure.h"
#include <string>

bool saveExecFile(const ExecFile& exec, const std::string& path);
bool loadExecFile(ExecFile& exec, const std::string& path);
