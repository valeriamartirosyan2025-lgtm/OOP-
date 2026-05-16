#include "VM_Interpreter.h"
#include <iostream>

void loadExecutable(VirtualProcessor& vp, const ExecFile& exec) {
    for (size_t i = 0; i < exec.codeSegment.size(); ++i) {
        vp.memory[CODE_START + i * 4] = (int32_t)exec.codeSegment[i].opCode;
    }
    for (size_t i = 0; i < exec.dataSegment.size(); ++i) {
        vp.memory[DATA_START + i] = exec.dataSegment[i];
    }
}

int32_t runVirtualEngine(VirtualProcessor& vp, const ExecFile& exec, int resultReg) {
    vp.pc = 0;
    while (vp.pc < (int)exec.codeSegment.size()) {
        vp.registers[0] = 0;
        const auto& inst = exec.codeSegment[vp.pc++];

        switch ((CommandID)inst.opCode) {
        case CommandID::SET_VAL:
            vp.registers[inst.rs1] = vp.memory[DATA_START + inst.offset];
            break;

        case CommandID::GET_VAR:
            vp.registers[inst.rs1] = vp.memory[DATA_START + inst.offset];
            break;

        case CommandID::PUT_VAR:
            vp.memory[DATA_START + inst.offset] = vp.registers[inst.rs1];
            break;

        case CommandID::ADD:
            vp.registers[inst.offset] = vp.registers[inst.rs1] + vp.registers[inst.rs2];
            break;

        case CommandID::SUB:
            vp.registers[inst.offset] = vp.registers[inst.rs1] - vp.registers[inst.rs2];
            break;

        case CommandID::MUL:
            vp.registers[inst.offset] = vp.registers[inst.rs1] * vp.registers[inst.rs2];
            break;

        case CommandID::DIV:
            if (vp.registers[inst.rs2] != 0)
                vp.registers[inst.offset] = vp.registers[inst.rs1] / vp.registers[inst.rs2];
            else
                vp.registers[inst.offset] = 0;
            break;

        case CommandID::BR_EQ:
            if (vp.registers[inst.rs1] == vp.registers[inst.rs2]) vp.pc += inst.offset;
            break;

        case CommandID::BR_GT:
            if (vp.registers[inst.rs1] > vp.registers[inst.rs2]) vp.pc += inst.offset;
            break;

        case CommandID::BR_LT:
            if (vp.registers[inst.rs1] < vp.registers[inst.rs2]) vp.pc += inst.offset;
            break;

        case CommandID::HALT:
            return vp.registers[inst.rs1];

        default:
            break;
        }
    }
    return vp.registers[resultReg];
}