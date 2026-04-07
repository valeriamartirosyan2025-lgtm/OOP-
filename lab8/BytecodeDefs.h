#pragma once
#include <vector>
#include <string>
#include <map>
#include <cstdint>

enum class CommandID : uint32_t {
    SET_VAL, GET_VAR, PUT_VAR, ADD_OP, SUB_OP, MUL_OP, DIV_OP,
    IS_EQ, IS_NEQ, IS_LT, IS_GT, IS_LTE, IS_GTE,
    JUMP_TO, JUMP_IF_ZERO, CALL_FUNC, RETURN_VAL, HALT
};

struct RawInstruction {
    uint32_t opCode : 5;
    uint32_t target : 9;
    uint32_t src1 : 9;
    uint32_t src2 : 9;
};

struct RoutineLayout {
    std::string title;
    std::vector<std::string> params;
    std::vector<RawInstruction> bytecode;
    std::vector<double> literals;
};

extern std::vector<RoutineLayout> globalRoutines;
extern std::map<std::string, int> routineMap;