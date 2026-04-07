#pragma once
#include "BytecodeDefs.h"
#include <vector>


double runVirtualEngine(
    const std::vector<RawInstruction>& script,
    const std::vector<double>& consts,
    std::vector<double>& mem,
    double* regs,
    int pc
);