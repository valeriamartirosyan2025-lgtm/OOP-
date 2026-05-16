#pragma once
#include <cstdint>
#include <vector>

enum class CommandID : uint32_t {
    ADD, SUB, MUL, DIV,
    SET_VAL,
    GET_VAR,
    PUT_VAR,
    BR_EQ, BR_NEQ, BR_LT, BR_GT, BR_LTE, BR_GTE,
    HALT
};

struct RawInstruction {
    uint32_t opCode;
    uint32_t rs1;
    uint32_t rs2;
    int32_t offset;
};

struct ExecFile {
    struct Header {
        uint32_t magicNumber = 0x163264;
        uint32_t codeSize;
        uint32_t dataSize;
    } header;

    std::vector<RawInstruction> codeSegment;
    std::vector<int32_t> dataSegment;
};
};