#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <map>

enum class CommandID : uint32_t {
    SET_VAL, GET_VAR, PUT_VAR,
    ADD_OP, SUB_OP, MUL_OP, DIV_OP,

    
    BR_EQ,   
    BR_NEQ,  
    BR_LT,   
    BR_GT,   
    BR_LTE,  
    BR_GTE,  

    HALT
};

struct RawInstruction {
    uint32_t opCode;  
    uint32_t rs1;    
    uint32_t rs2;     
    int32_t offset;   
};

struct RoutineLayout {
    std::string title;
    std::vector<std::string> params;
    std::vector<RawInstruction> bytecode;
    std::vector<double> literals;
};

extern std::vector<RoutineLayout> globalRoutines;
extern std::map<std::string, int> routineMap;