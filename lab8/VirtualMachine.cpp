#include "VirtualMachine.h"
#include <iostream>
#include <cmath>

double runVirtualEngine(const std::vector<RawInstruction>& script, const std::vector<double>& consts, std::vector<double>& mem, double* regs, int pc) {
    while (pc < (int)script.size()) {
        const auto& inst = script[pc++];

        switch ((CommandID)inst.opCode) {
        case CommandID::SET_VAL:
            regs[inst.target] = consts[inst.src1];
            break;

        case CommandID::GET_VAR:
            regs[inst.target] = mem[inst.src1];
            break;

        case CommandID::PUT_VAR:
            mem[inst.target] = regs[inst.src1];
            break;

        case CommandID::ADD_OP:
            regs[inst.target] = regs[inst.src1] + regs[inst.src2];
            break;

        case CommandID::SUB_OP:
            regs[inst.target] = regs[inst.src1] - regs[inst.src2];
            break;

        case CommandID::MUL_OP:
            regs[inst.target] = regs[inst.src1] * regs[inst.src2];
            break;

        case CommandID::DIV_OP:
            if (regs[inst.src2] != 0)
                regs[inst.target] = regs[inst.src1] / regs[inst.src2];
            else
                regs[inst.target] = 0; s
            break;

        case CommandID::IS_EQ:
            regs[inst.target] = (regs[inst.src1] == regs[inst.src2]) ? 1.0 : 0.0;
            break;

        case CommandID::HALT:
            return regs[inst.target];

        default:
            break;
        }
    }
    return 0.0;
}