#pragma once
#include "ExecFileStructure.h"
#include <map>
#include <string>

// Text assembler: mov r1, r2 | add r1, r2 | push r1 | call name | ...
bool assembleFile(const std::string& path, ExecFile& out,
                  std::map<std::string, int>& functionLabels);
