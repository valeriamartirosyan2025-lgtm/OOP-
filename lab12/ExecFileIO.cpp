#include "ExecFileIO.h"
#include <fstream>

bool saveExecFile(const ExecFile& exec, const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    out.write(reinterpret_cast<const char*>(&exec.header), sizeof(exec.header));
    for (const auto& inst : exec.codeSegment) {
        out.write(reinterpret_cast<const char*>(&inst.opCode), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&inst.rs1), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&inst.rs2), sizeof(uint32_t));
        out.write(reinterpret_cast<const char*>(&inst.offset), sizeof(int32_t));
        uint32_t len = (uint32_t)inst.labelTarget.size();
        out.write(reinterpret_cast<const char*>(&len), sizeof(uint32_t));
        if (len) out.write(inst.labelTarget.data(), len);
    }
    for (int32_t w : exec.dataSegment)
        out.write(reinterpret_cast<const char*>(&w), sizeof(int32_t));
    return true;
}

bool loadExecFile(ExecFile& exec, const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    in.read(reinterpret_cast<char*>(&exec.header), sizeof(exec.header));
    exec.codeSegment.clear();
    for (uint32_t i = 0; i < exec.header.codeWords; ++i) {
        RawInstruction inst;
        in.read(reinterpret_cast<char*>(&inst.opCode), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&inst.rs1), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&inst.rs2), sizeof(uint32_t));
        in.read(reinterpret_cast<char*>(&inst.offset), sizeof(int32_t));
        uint32_t len = 0;
        in.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
        if (len) {
            inst.labelTarget.resize(len);
            in.read(inst.labelTarget.data(), len);
        }
        exec.codeSegment.push_back(inst);
    }
    exec.dataSegment.resize(exec.header.dataWords);
    for (uint32_t i = 0; i < exec.header.dataWords; ++i)
        in.read(reinterpret_cast<char*>(&exec.dataSegment[i]), sizeof(int32_t));
    return true;
}
