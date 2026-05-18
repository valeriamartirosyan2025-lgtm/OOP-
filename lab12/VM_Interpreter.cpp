#include "VM_Interpreter.h"
#include <iostream>

namespace {
const int REG_SP = 14;
const int REG_BP = 15;
const int REG_TO_REG = -3;

int32_t readMem(VirtualProcessor& vp, int addr) {
    if (addr < 0 || addr >= MEM_SIZE) return 0;
    return vp.memory[addr];
}

void writeMem(VirtualProcessor& vp, int addr, int32_t v) {
    if (addr >= 0 && addr < MEM_SIZE) vp.memory[addr] = v;
}

bool branchTaken(CommandID br, int flags) {
    switch (br) {
    case CommandID::BR_EQ: return flags == 0;
    case CommandID::BR_NE: return flags != 0;
    case CommandID::BR_LT: return flags < 0;
    case CommandID::BR_LE: return flags <= 0;
    case CommandID::BR_GT: return flags > 0;
    case CommandID::BR_GE: return flags >= 0;
    case CommandID::BR_LTU: return (uint32_t)flags == 1;
    case CommandID::BR_GTU: return (uint32_t)flags == 2;
    default: return false;
    }
}

bool executeOne(VirtualProcessor& vp, const ExecFile& exec,
                const std::map<std::string, int>& fMap) {
    if (vp.halted) return false;
    int total = (int)exec.codeSegment.size();
    int idx = vp.pc / WORD;
    if (idx < 0 || idx >= total) {
        vp.halted = true;
        return false;
    }

    const RawInstruction& inst = exec.codeSegment[idx];
    vp.registers[0] = 0;

    switch ((CommandID)inst.opCode) {
    case CommandID::MOV:
        if (inst.offset == REG_TO_REG)
            vp.registers[inst.rs1] = vp.registers[inst.rs2];
        else if (inst.rs2 == REG_BP)
            vp.registers[inst.rs1] = readMem(vp, vp.registers[REG_BP] + inst.offset);
        else
            vp.registers[inst.rs1] = readMem(vp, DATA_START + inst.offset);
        vp.pc += WORD;
        break;

    case CommandID::STORE:
        if (inst.rs2 == REG_BP)
            writeMem(vp, vp.registers[REG_BP] + inst.offset, vp.registers[inst.rs1]);
        else
            writeMem(vp, DATA_START + inst.offset, vp.registers[inst.rs1]);
        vp.pc += WORD;
        break;

    case CommandID::ADD:
        if (inst.rs1 == REG_SP)
            vp.registers[REG_SP] += inst.offset;
        else
            vp.registers[inst.rs1] += (inst.rs2 ? vp.registers[inst.rs2] : inst.offset);
        vp.pc += WORD;
        break;

    case CommandID::SUB:
        if (inst.rs1 == REG_SP)
            vp.registers[REG_SP] -= inst.offset;
        else if (inst.rs2)
            vp.registers[inst.rs1] -= vp.registers[inst.rs2];
        else
            vp.registers[inst.rs1] -= inst.offset;
        vp.pc += WORD;
        break;

    case CommandID::MUL:
        vp.registers[inst.rs1] *= vp.registers[inst.rs2];
        vp.pc += WORD;
        break;

    case CommandID::DIV:
        if (vp.registers[inst.rs2] != 0)
            vp.registers[inst.rs1] /= vp.registers[inst.rs2];
        vp.pc += WORD;
        break;

    case CommandID::CMP: {
        int32_t a = vp.registers[inst.rs1];
        int32_t b = vp.registers[inst.rs2];
        if (a < b) vp.cmpFlags = -1;
        else if (a > b) vp.cmpFlags = 1;
        else vp.cmpFlags = 0;
        vp.pc += WORD;
        break;
    }

    case CommandID::JMP:
        vp.pc = (idx + inst.offset) * WORD;
        break;

    case CommandID::BR_EQ:
    case CommandID::BR_NE:
    case CommandID::BR_LT:
    case CommandID::BR_LE:
    case CommandID::BR_GT:
    case CommandID::BR_GE:
    case CommandID::BR_LTU:
    case CommandID::BR_GTU:
        if (branchTaken((CommandID)inst.opCode, vp.cmpFlags))
            vp.pc = (idx + inst.offset) * WORD;
        else
            vp.pc += WORD;
        break;

    case CommandID::PUSH:
        vp.registers[REG_SP] -= WORD;
        writeMem(vp, vp.registers[REG_SP], vp.registers[inst.rs1]);
        vp.pc += WORD;
        break;

    case CommandID::POP:
        vp.registers[inst.rs1] = readMem(vp, vp.registers[REG_SP]);
        vp.registers[REG_SP] += WORD;
        vp.pc += WORD;
        break;

    case CommandID::CALL: {
        int returnAddr = (idx + 1) * WORD;
        vp.registers[REG_SP] -= WORD;
        writeMem(vp, vp.registers[REG_SP], returnAddr);
        if (!inst.labelTarget.empty()) {
            auto it = fMap.find(inst.labelTarget);
            vp.pc = (it != fMap.end()) ? it->second : inst.offset;
        } else
            vp.pc = inst.offset;
        break;
    }

    case CommandID::RET:
        vp.pc = readMem(vp, vp.registers[REG_SP]);
        vp.registers[REG_SP] += WORD;
        break;

    case CommandID::HALT:
        vp.halted = true;
        return false;

    default:
        vp.pc += WORD;
        break;
    }
    return !vp.halted;
}
} // namespace

void loadExecutable(VirtualProcessor& vp, const ExecFile& exec) {
    for (size_t i = 0; i < exec.codeSegment.size(); ++i) {
        const auto& in = exec.codeSegment[i];
        int base = CODE_START + (int)i * WORD * 4;
        vp.memory[CODE_START + (int)i * WORD] = (int32_t)in.opCode;
        vp.memory[CODE_START + (int)i * WORD + 1] = (int32_t)in.rs1;
        vp.memory[CODE_START + (int)i * WORD + 2] = (int32_t)in.rs2;
        vp.memory[CODE_START + (int)i * WORD + 3] = in.offset;
        (void)base;
    }
    for (size_t i = 0; i < exec.dataSegment.size(); ++i)
        vp.memory[DATA_START + (int)i * WORD] = exec.dataSegment[i];

    vp.registers[REG_SP] = STACK_TOP;
    vp.registers[REG_BP] = STACK_TOP;
    vp.pc = exec.header.entryPC;
    vp.halted = false;
    vp.cmpFlags = 0;
}

int32_t runVirtualEngine(VirtualProcessor& vp, const ExecFile& exec,
                         int resultReg, const std::map<std::string, int>& fMap) {
    if (vp.pc == 0) vp.pc = exec.header.entryPC;
    vp.halted = false;
    while (stepVirtualEngine(vp, exec, fMap)) {}
    return vp.registers[resultReg];
}

bool stepVirtualEngine(VirtualProcessor& vp, const ExecFile& exec,
                       const std::map<std::string, int>& fMap) {
    return executeOne(vp, exec, fMap);
}
