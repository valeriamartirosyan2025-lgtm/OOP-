#pragma once
#include "ExecFileStructure.h"
#include <map>
#include <string>

// Memory map (whiteboard): Code @ 0, Data, Stack grows down from top
const int XLEN = 32;
const int WORD = 4;
const int MEM_SIZE = 65536;
const int CODE_START = 0;
const int DATA_START = 4096;
const int STACK_TOP = MEM_SIZE - WORD;

struct VirtualProcessor {
    int32_t registers[16] = {};
    int32_t memory[MEM_SIZE] = {};
    int pc = 0;
    int cmpFlags = 0; // -1, 0, 1 from last CMP
    bool halted = false;
};

void loadExecutable(VirtualProcessor& vp, const ExecFile& exec);
int32_t runVirtualEngine(VirtualProcessor& vp, const ExecFile& exec,
                         int resultReg, const std::map<std::string, int>& fMap);
bool stepVirtualEngine(VirtualProcessor& vp, const ExecFile& exec,
                       const std::map<std::string, int>& fMap);
