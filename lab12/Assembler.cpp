#include "Assembler.h"
#include <fstream>
#include <sstream>
#include <unordered_map>

static int parseReg(const std::string& s) {
    if (s.size() >= 2 && (s[0] == 'r' || s[0] == 'R'))
        return std::stoi(s.substr(1));
    if (s == "sp") return 14;
    if (s == "bp") return 15;
    return 0;
}

bool assembleFile(const std::string& path, ExecFile& out,
                  std::map<std::string, int>& functionLabels) {
    std::ifstream in(path);
    if (!in) return false;

    std::unordered_map<std::string, int> labels;
    std::vector<std::pair<int, std::string>> pending;
    std::string line;

    auto emit = [&](RawInstruction i) {
        return (int)out.codeSegment.size();
    };

    while (std::getline(in, line)) {
        auto hash = line.find('#');
        if (hash != std::string::npos) line = line.substr(0, hash);
        std::istringstream iss(line);
        std::string word;
        if (!(iss >> word)) continue;

        if (word.back() == ':') {
            labels[word.substr(0, word.size() - 1)] = (int)out.codeSegment.size();
            continue;
        }

        std::vector<std::string> args;
        std::string a;
        while (iss >> a) args.push_back(a);

        RawInstruction inst{};
        if (word == "mov") {
            inst.opCode = (uint32_t)CommandID::MOV;
            inst.rs1 = parseReg(args[0]);
            if (args.size() > 1 && args[1][0] == 'r')
                { inst.rs2 = parseReg(args[1]); inst.offset = -3; }
            else
                inst.offset = std::stoi(args[1]);
        } else if (word == "add") {
            inst = { (uint32_t)CommandID::ADD, (uint32_t)parseReg(args[0]), (uint32_t)parseReg(args[1]), 0 };
        } else if (word == "push") {
            inst = { (uint32_t)CommandID::PUSH, (uint32_t)parseReg(args[0]), 0, 0 };
        } else if (word == "call") {
            inst = { (uint32_t)CommandID::CALL, 0, 0, 0 };
            inst.labelTarget = args[0];
        } else if (word == "ret") {
            inst = { (uint32_t)CommandID::RET, 0, 0, 0 };
        } else if (word == "halt") {
            inst = { (uint32_t)CommandID::HALT, 0, 0, 0 };
        } else if (word == "jmp") {
            inst = { (uint32_t)CommandID::JMP, 0, 0, 0 };
            pending.push_back({ emit(inst), args[0] });
            out.codeSegment.push_back(inst);
            continue;
        } else continue;

        out.codeSegment.push_back(inst);
    }

    for (const auto& p : pending) {
        auto it = labels.find(p.second);
        if (it != labels.end())
            out.codeSegment[p.first].offset = it->second - p.first;
    }
    for (const auto& kv : labels)
        functionLabels[kv.first] = kv.second * 4;

    out.header.codeWords = (uint32_t)out.codeSegment.size();
    return true;
}
