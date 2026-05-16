#include "BytecodeDefs.h"

std::vector<RoutineLayout> globalRoutines;
std::map<std::string, int> routineMap;

std::string getCommandName(CommandID id) {
    static const std::string names[] = {
        "SET_VAL", "GET_VAR", "PUT_VAR", "ADD", "SUB", "MUL", "DIV",
        "BR_EQ", "BR_NEQ", "BR_LT", "BR_GT", "BR_LTE", "BR_GTE", "HALT"
    };
    return names[(int)id];
}