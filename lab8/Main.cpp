#include "Scanner.h"
#include "SyntaxEngine.h"
#include "Compiler.h"
#include "VirtualMachine.h"
#include <iostream>
#include <string>
#include <map>

int main() {
    std::cout << "------------" << std::endl;
    std::cout << " -----------  " << std::endl;
    std::cout << "------------" << std::endl;

    std::string sourceLine;
    std::cout << "\nEnter Expression: ";
    std::getline(std::cin, sourceLine);

    if (sourceLine.empty()) return 0;

    try {
       
        runScanner(sourceLine);

      
        auto astRoot = constructAST();
        if (!astRoot) {
            std::cerr << "Error: Could not build AST." << std::endl;
            return 1;
        }

     
        std::vector<RawInstruction> bytecode;
        std::vector<double> constantPool;
        std::map<std::string, int> symbolTable;
        int registerCount = 0;

        int finalResultReg = generateBytecode(astRoot.get(), bytecode, constantPool, registerCount, symbolTable);

        if (finalResultReg != -1) {
            
            bytecode.push_back({ (uint32_t)CommandID::HALT, (uint32_t)finalResultReg, 0, 0 });

            
            std::vector<double> memory(256, 0.0); 
            double cpuRegisters[1024] = { 0.0 }; 

            std::cout << "\n[System] Starting Virtual Machine Execution..." << std::endl;
            double finalValue = runVirtualEngine(bytecode, constantPool, memory, cpuRegisters, 0);

            std::cout << "[Result] Output: " << finalValue << std::endl;
        }

    }
    catch (const std::exception& ex) {
        std::cerr << "\n[Fatal Error]: " << ex.what() << std::endl;
    }

    std::cout << "\nPress Enter to close...";
    std::cin.get();
    return 0;
}