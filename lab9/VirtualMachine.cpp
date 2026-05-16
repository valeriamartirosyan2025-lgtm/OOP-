#include "VirtualMachine.h"
#include <iostream>

double runVirtualEngine(const std::vector<RawInstruction>& script, const std::vector<double>& consts, std::vector<double>& mem, double* regs, int pc) {
    while (pc < (int)script.size()) {
        const auto& inst = script[pc++]; 

        switch ((CommandID)inst.opCode) {
        case CommandID::SET_VAL:
            regs[inst.rs1] = consts[inst.rs2];
            break;

        case CommandID::GET_VAR:
            regs[inst.rs1] = mem[inst.rs2];
            break;

        case CommandID::PUT_VAR:
            mem[inst.rs1] = regs[inst.rs2];
            break;

        case CommandID::ADD_OP:
            regs[inst.rs1] = regs[inst.rs2] + regs[inst.offset]; 
            break;

        case CommandID::SUB_OP:
            regs[inst.rs1] = regs[inst.rs2] - regs[inst.offset];
            break;

        case CommandID::MUL_OP:
            regs[inst.rs1] = regs[inst.rs2] * regs[inst.offset];
            break;

        case CommandID::DIV_OP:
            if (regs[inst.offset] != 0)
                regs[inst.rs1] = regs[inst.rs2] / regs[inst.offset];
            else
                regs[inst.rs1] = 0;
            break;

            
        case CommandID::BR_EQ:
            if (regs[inst.rs1] == regs[inst.rs2]) {
                pc += inst.offset; 
            }
            break;

        case CommandID::BR_GT:
            if (regs[inst.rs1] > regs[inst.rs2]) {
                pc += inst.offset;
            }
            break;

        case CommandID::BR_LT:
            if (regs[inst.rs1] < regs[inst.rs2]) {
                pc += inst.offset;
            }
            break;

        case CommandID::BR_GTE:
            if (regs[inst.rs1] >= regs[inst.rs2]) {
                pc += inst.offset;
            }
            break;

        case CommandID::BR_LTE:
            if (regs[inst.rs1] <= regs[inst.rs2]) {
                pc += inst.offset;
            }
            break;

        case CommandID::HALT:
            return regs[inst.rs1];

        default:
            break;
        }
    }
    return 0.0;
}