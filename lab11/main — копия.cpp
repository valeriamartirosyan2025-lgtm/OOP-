#include "Scanner.h"
#include "Parser.h"
#include "CodeGenerator.h"
#include "VM_Interpreter.h"
#include <iostream>
#include <string>
#include <map>

int main() {
    std::cout << "=== RISC-V Byte-Aligned VM Architecture ===" << std::endl;
    std::string sourceLine;
    std::cout << "Enter Expression (e.g., 5 > 3): ";
    std::getline(std::cin, sourceLine);

    if (sourceLine.empty()) return 0;

    try {
        runScanner(sourceLine);
        auto astRoot = constructAST();

        ExecFile binaryFile;
        std::map<std::string, int> symbolTable;
        int regTracker = 1;

        int finalReg = generateBytecode(astRoot.get(), binaryFile, regTracker, symbolTable);

        binaryFile.header.codeSize = binaryFile.codeSegment.size() * 4;
        binaryFile.header.dataSize = binaryFile.dataSegment.size() * 4;

        VirtualProcessor riscvCPU;
        loadExecutable(riscvCPU, binaryFile);

        std::cout << "[Loader] Exec File loaded. Memory alignment set to 4 Bytes." << std::endl;
        int32_t output = runVirtualEngine(riscvCPU, binaryFile, finalReg);

        std::cout << "[Processor Output]: " << output << std::endl;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
    }
    return 0;
}