#pragma once
#include "ExecFileStructure.h"
#include "VM_Interpreter.h"
#include <map>
#include <string>

class Debugger {
public:
    Debugger(VirtualProcessor& cpu, const ExecFile& exec,
             const std::map<std::string, int>& functions);

    void run();
    void cmdHelp() const;

private:
    void printState() const;
    void printRegisters() const;
    void disassemble(int count = 8) const;

    VirtualProcessor& cpu_;
    const ExecFile& exec_;
    const std::map<std::string, int>& functions_;
};
