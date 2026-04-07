v#include "BytecodeDefs.h"

std::vector<RoutineLayout> globalRoutines;
std::map<std::string, int> routineMap;

std::string getCommandName(CommandID id) {
    static const std::string names[] = { "SET_VAL", "GET_VAR", "PUT_VAR", "ADD", "SUB", "MUL", "DIV" /*...*/ };
    return names[(int)id];
}