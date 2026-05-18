#pragma once
#include <cstdint>
#include <string>
#include <vector>

// 32-bit RISC-style VM ISA (lab whiteboard: br, jmp, stack frames)
enum class CommandID : uint32_t {
    MOV, STORE,
    ADD, SUB, MUL, DIV,
    CMP,
    JMP,
    BR_EQ, BR_NE, BR_LT, BR_LE, BR_GT, BR_GE,
    BR_LTU, BR_GTU,
    PUSH, POP,
    CALL, RET,
    HALT
};

struct RawInstruction {
    uint32_t opCode = 0;
    uint32_t rs1 = 0;
    uint32_t rs2 = 0;
    int32_t offset = 0;
    std::string labelTarget;
    int patchLabel = -1; // unresolved label id
};

struct ExecFile {
    struct Header {
        uint32_t magicNumber = 0x4C423132; // "LB12"
        uint32_t codeWords = 0;
        uint32_t dataWords = 0;
        uint32_t entryPC = 0;
        uint32_t xlen = 32;
    } header;

    std::vector<RawInstruction> codeSegment;
    std::vector<int32_t> dataSegment;
    std::string entryFunction = "main";
};
