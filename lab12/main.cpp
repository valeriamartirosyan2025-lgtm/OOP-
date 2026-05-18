#include "AST.h"
#include "CodeGenerator.h"
#include "Debugger.h"
#include "ExecFileIO.h"
#include "Lexer.h"
#include "Parser.h"
#include "VM_Interpreter.h"
#include <fstream>
#include <iostream>
#include <sstream>

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open: " + path);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void printUsage() {
    std::cout << "Lab12 Compiler + VM\n"
              << "  OOP_LAB12 compile <source.src> [out.exec]\n"
              << "  OOP_LAB12 run <source.src>\n"
              << "  OOP_LAB12 debug <source.src>\n"
              << "  OOP_LAB12 (no args) — built-in demo\n";
}

int main(int argc, char* argv[]) {
    try {
        std::string sourcePath = "programs/demo.src";
        std::string mode = "run";
        std::string outPath = "output.exec";

        if (argc >= 2) {
            std::string arg1 = argv[1];
            if (arg1 == "compile" || arg1 == "run" || arg1 == "debug") {
                mode = arg1;
                if (argc >= 3) sourcePath = argv[2];
                if (argc >= 4 && mode == "compile") outPath = argv[3];
            } else {
                sourcePath = arg1;
                if (argc >= 3) mode = argv[2];
            }
        }

        std::string source = readFile(sourcePath);
        Lexer lex(source);
        Parser parser(lex);
        auto ast = parser.parseProgram();

        CompileResult compiled = compileProgram(ast.get());
        saveExecFile(compiled.exec, outPath);

        std::cout << "=== Compiler Pipeline ===\n"
                  << "Source: " << sourcePath << "\n"
                  << "Functions: ";
        for (const auto& f : compiled.functionTable)
            std::cout << f.first << "@" << f.second << " ";
        std::cout << "\nCode words: " << compiled.exec.codeSegment.size()
                  << "  Data words: " << compiled.exec.dataSegment.size() << "\n";

        if (mode == "compile") {
            std::cout << "Wrote " << outPath << "\n";
            return 0;
        }

        VirtualProcessor cpu;
        loadExecutable(cpu, compiled.exec);
        cpu.pc = compiled.exec.header.entryPC;
        if (mode == "debug") {
            Debugger dbg(cpu, compiled.exec, compiled.functionTable);
            dbg.run();
            return 0;
        }

        int32_t result = runVirtualEngine(cpu, compiled.exec, 1, compiled.functionTable);
        std::cout << "[Result in R1]: " << result << " (expected 30 for demo)\n";
        return 0;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        if (argc <= 1) printUsage();
        return 1;
    }
}
