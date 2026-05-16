#pragma once
#include "ExecFileStructure.h"

const int MEM_SIZE = 4096;
const int CODE_START = 0;
const int DATA_START = 1024;

struct VirtualProcessor {
    int32_t registers[16] = { 0 };
    int32_t memory[MEM_SIZE] = { 0 };
    int pc = 0;                  
};

void loadExecutable(VirtualProcessor& vp, const ExecFile& exec);
int32_t runVirtualEngine(VirtualProcessor& vp, const ExecFile& exec, int resultReg);