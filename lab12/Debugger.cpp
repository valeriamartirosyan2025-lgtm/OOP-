#include "Debugger.h"
#include <iostream>
#include <sstream>
#include <string>

Debugger::Debugger(VirtualProcessor& cpu, const ExecFile& exec,
                   const std::map<std::string, int>& functions)
    : cpu_(cpu), exec_(exec), functions_(functions) {}

void Debugger::cmdHelp() const {
    std::cout << "Commands: s(step) c(continue) r(registers) m(memory state) "
                 "d(disasm) b <func> (breakpoint pc) q(quit)\n";
}

void Debugger::printRegisters() const {
    std::cout << "PC=" << cpu_.pc << " SP=" << cpu_.registers[14]
              << " BP=" << cpu_.registers[15] << " FLAGS=" << cpu_.cmpFlags << "\n";
    for (int i = 1; i <= 13; ++i)
        std::cout << " R" << i << "=" << cpu_.registers[i];
    std::cout << "\n";
}

void Debugger::disassemble(int count) const {
    int idx = cpu_.pc / 4;
    for (int i = 0; i < count && idx + i < (int)exec_.codeSegment.size(); ++i) {
        const auto& in = exec_.codeSegment[idx + i];
        std::cout << "[" << (idx + i) * 4 << "] op=" << in.opCode
                  << " r1=" << in.rs1 << " r2=" << in.rs2
                  << " off=" << in.offset;
        if (!in.labelTarget.empty()) std::cout << " @" << in.labelTarget;
        std::cout << "\n";
    }
}

void Debugger::printState() const {
    printRegisters();
    disassemble(4);
}

void Debugger::run() {
    std::cout << "=== VM Debugger ===\n";
    cmdHelp();
    std::string line;
    while (std::cout << "dbg> " && std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        if (cmd == "q" || cmd == "quit") break;
        if (cmd == "h" || cmd == "help") { cmdHelp(); continue; }
        if (cmd == "r" || cmd == "reg") { printRegisters(); continue; }
        if (cmd == "m" || cmd == "state") { printState(); continue; }
        if (cmd == "d" || cmd == "dis") { disassemble(12); continue; }
        if (cmd == "s" || cmd == "step") {
            stepVirtualEngine(cpu_, exec_, functions_);
            printState();
            if (cpu_.halted) {
                std::cout << "HALT. R1=" << cpu_.registers[1] << "\n";
                break;
            }
            continue;
        }
        if (cmd == "c" || cmd == "cont") {
            while (!cpu_.halted) stepVirtualEngine(cpu_, exec_, functions_);
            std::cout << "HALT. Result R1=" << cpu_.registers[1] << "\n";
            break;
        }
        if (cmd == "b") {
            std::string fn;
            iss >> fn;
            auto it = functions_.find(fn);
            if (it != functions_.end()) {
                cpu_.pc = it->second;
                std::cout << "Breakpoint at " << fn << " PC=" << cpu_.pc << "\n";
            }
            continue;
        }
        std::cout << "Unknown command\n";
    }
}
