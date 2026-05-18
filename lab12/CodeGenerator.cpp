#include "CodeGenerator.h"

namespace {
const int REG_RET = 1;
const int REG_SP = 14;
const int REG_BP = 15;
const int REG_TO_REG = -3;

class CodeGenerator {
public:
    explicit CodeGenerator(ExecFile& out) : exec_(out) {}

    CompileResult compile(const ExpressionNode* program) {
        for (const auto& decl : program->nested) {
            if (decl->kind == NodeType::VarDecl)
                initGlobal(decl.get());
            else if (decl->kind == NodeType::FuncDecl)
                compileFunction(decl.get());
        }

        resolvePatches();

        RawInstruction callMain;
        callMain.opCode = (uint32_t)CommandID::CALL;
        callMain.labelTarget = exec_.entryFunction;
        int callIdx = emit(callMain);
        auto itMain = functions_.find(exec_.entryFunction);
        if (itMain != functions_.end())
            exec_.codeSegment[callIdx].offset = itMain->second;
        emit({ (uint32_t)CommandID::HALT, 0, 0, 0 });

        for (auto& inst : exec_.codeSegment) {
            if (inst.opCode == (uint32_t)CommandID::CALL && !inst.labelTarget.empty()) {
                auto it = functions_.find(inst.labelTarget);
                if (it != functions_.end())
                    inst.offset = it->second;
            }
        }

        exec_.header.codeWords = (uint32_t)exec_.codeSegment.size();
        exec_.header.dataWords = (uint32_t)exec_.dataSegment.size();
        exec_.header.entryPC = callIdx * 4;

        CompileResult r;
        r.exec = exec_;
        r.functionTable = functions_;
        return r;
    }

private:
    ExecFile exec_;
    std::map<std::string, int> functions_;
    std::map<std::string, int> globals_;
    std::map<std::string, int> locals_;
    int regTracker_ = 2;
    int localBytes_ = 0;
    int nextLocal_ = -4;
    int labelId_ = 0;
    std::map<int, int> labels_;
    std::vector<std::pair<int, int>> patches_;

    int emit(RawInstruction i) {
        int idx = (int)exec_.codeSegment.size();
        exec_.codeSegment.push_back(std::move(i));
        return idx;
    }

    int newLabel() { return labelId_++; }
    void bindLabel(int id) { labels_[id] = (int)exec_.codeSegment.size(); }

    void resolvePatches() {
        for (const auto& p : patches_) {
            int from = p.first;
            int to = labels_[p.second];
            exec_.codeSegment[from].offset = to - from;
        }
    }

    void patchJump(int instrIdx, int labelId) {
        patches_.push_back({ instrIdx, labelId });
    }

    void resetLocals() {
        locals_.clear();
        localBytes_ = 0;
        nextLocal_ = -4;
        regTracker_ = 2;
    }

    int allocLocal(const std::string& name) {
        if (!locals_.count(name)) {
            locals_[name] = nextLocal_;
            nextLocal_ -= 4;
            localBytes_ += 4;
        }
        return locals_[name];
    }

    int emitLiteral(int32_t v) {
        int rd = regTracker_++;
        int idx = (int)exec_.dataSegment.size() * 4;
        exec_.dataSegment.push_back(v);
        emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, 0, idx });
        return rd;
    }

    CommandID cmpBranch(CmpOp op) {
        switch (op) {
        case CmpOp::EQ: return CommandID::BR_EQ;
        case CmpOp::NE: return CommandID::BR_NE;
        case CmpOp::LT: return CommandID::BR_LT;
        case CmpOp::LE: return CommandID::BR_LE;
        case CmpOp::GT: return CommandID::BR_GT;
        case CmpOp::GE: return CommandID::BR_GE;
        }
        return CommandID::BR_EQ;
    }

    CommandID invertBranch(CmpOp op) {
        switch (op) {
        case CmpOp::EQ: return CommandID::BR_NE;
        case CmpOp::NE: return CommandID::BR_EQ;
        case CmpOp::LT: return CommandID::BR_GE;
        case CmpOp::LE: return CommandID::BR_GT;
        case CmpOp::GT: return CommandID::BR_LE;
        case CmpOp::GE: return CommandID::BR_LT;
        }
        return CommandID::BR_NE;
    }

    void initGlobal(const ExpressionNode* decl) {
        if (!globals_.count(decl->textValue)) {
            globals_[decl->textValue] = (int)exec_.dataSegment.size() * 4;
            exec_.dataSegment.push_back(0);
        }
        if (decl->rightSide) {
            int v = genExpr(decl->rightSide.get());
            emit({ (uint32_t)CommandID::STORE, (uint32_t)v, 0, globals_[decl->textValue] });
        }
    }

    void compileFunction(const ExpressionNode* fn) {
        resetLocals();
        functions_[fn->textValue] = (int)exec_.codeSegment.size() * 4;

        for (size_t i = 0; i < fn->argList.size(); ++i)
            locals_[fn->argList[i]] = 8 + (int)i * 4;

        emit({ (uint32_t)CommandID::PUSH, (uint32_t)REG_BP, 0, 0 });
        emit({ (uint32_t)CommandID::MOV, (uint32_t)REG_BP, (uint32_t)REG_SP, REG_TO_REG });

        if (fn->body) {
            for (const auto& s : fn->body->nested)
                if (s->kind == NodeType::VarDecl && s->storage == VarStorage::Local)
                    allocLocal(s->textValue);
        }
        if (localBytes_ > 0)
            emit({ (uint32_t)CommandID::SUB, (uint32_t)REG_SP, 0, localBytes_ });

        bool hasReturn = false;
        if (fn->body) {
            for (const auto& s : fn->body->nested) {
                if (s->kind == NodeType::Return) hasReturn = true;
                genStmt(s.get());
            }
        }
        if (!hasReturn) {
            emit({ (uint32_t)CommandID::MOV, (uint32_t)REG_RET, 0, REG_TO_REG });
            emitEpilogue();
        }
    }

    void emitEpilogue() {
        emit({ (uint32_t)CommandID::MOV, (uint32_t)REG_SP, (uint32_t)REG_BP, REG_TO_REG });
        emit({ (uint32_t)CommandID::POP, (uint32_t)REG_BP, 0, 0 });
        emit({ (uint32_t)CommandID::RET, 0, 0, 0 });
    }

    int loadVar(const std::string& name) {
        int rd = regTracker_++;
        if (locals_.count(name))
            emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, (uint32_t)REG_BP, locals_[name] });
        else {
            if (!globals_.count(name)) {
                globals_[name] = (int)exec_.dataSegment.size() * 4;
                exec_.dataSegment.push_back(0);
            }
            emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, 0, globals_[name] });
        }
        return rd;
    }

    void storeVar(const std::string& name, int rs) {
        if (locals_.count(name))
            emit({ (uint32_t)CommandID::STORE, (uint32_t)rs, (uint32_t)REG_BP, locals_[name] });
        else {
            if (!globals_.count(name)) {
                globals_[name] = (int)exec_.dataSegment.size() * 4;
                exec_.dataSegment.push_back(0);
            }
            emit({ (uint32_t)CommandID::STORE, (uint32_t)rs, 0, globals_[name] });
        }
    }

    void genConditionJump(const ExpressionNode* cond, int falseLabel) {
        if (cond && cond->kind == NodeType::LogicCmp) {
            int l = genExpr(cond->leftSide.get());
            int r = genExpr(cond->rightSide.get());
            emit({ (uint32_t)CommandID::CMP, (uint32_t)l, (uint32_t)r, 0 });
            RawInstruction br;
            br.opCode = (uint32_t)invertBranch(cond->cmpOp);
            br.rs1 = (uint32_t)l;
            br.rs2 = (uint32_t)r;
            patchJump(emit(br), falseLabel);
        } else {
            int v = genExpr(cond);
            int z = emitLiteral(0);
            emit({ (uint32_t)CommandID::CMP, (uint32_t)v, (uint32_t)z, 0 });
            RawInstruction br;
            br.opCode = (uint32_t)CommandID::BR_EQ;
            br.rs1 = (uint32_t)v;
            br.rs2 = (uint32_t)z;
            br.patchLabel = falseLabel;
            patchJump(emit(br), falseLabel);
        }
    }

    int genExpr(const ExpressionNode* n) {
        if (!n) return emitLiteral(0);
        switch (n->kind) {
        case NodeType::Literal:
            return emitLiteral((int32_t)n->numericData);
        case NodeType::Symbol:
            return loadVar(n->textValue);
        case NodeType::UnaryOp: {
            int v = genExpr(n->leftSide.get());
            int rd = regTracker_++;
            emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, 0, REG_TO_REG });
            emit({ (uint32_t)CommandID::SUB, (uint32_t)rd, (uint32_t)v, 0 });
            return rd;
        }
        case NodeType::BinaryOp: {
            int l = genExpr(n->leftSide.get());
            int r = genExpr(n->rightSide.get());
            int rd = regTracker_++;
            emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, (uint32_t)l, REG_TO_REG });
            CommandID op = CommandID::ADD;
            if (n->textValue == "-") op = CommandID::SUB;
            else if (n->textValue == "*") op = CommandID::MUL;
            else if (n->textValue == "/") op = CommandID::DIV;
            emit({ (uint32_t)op, (uint32_t)rd, (uint32_t)r, 0 });
            return rd;
        }
        case NodeType::FuncInvoke: {
            for (int i = (int)n->nested.size() - 1; i >= 0; --i) {
                int a = genExpr(n->nested[i].get());
                emit({ (uint32_t)CommandID::PUSH, (uint32_t)a, 0, 0 });
            }
            RawInstruction c;
            c.opCode = (uint32_t)CommandID::CALL;
            c.labelTarget = n->textValue;
            emit(c);
            int clean = (int)n->nested.size() * 4;
            if (clean)
                emit({ (uint32_t)CommandID::ADD, (uint32_t)REG_SP, 0, clean });
            int rd = regTracker_++;
            emit({ (uint32_t)CommandID::MOV, (uint32_t)rd, (uint32_t)REG_RET, REG_TO_REG });
            return rd;
        }
        default:
            return emitLiteral(0);
        }
    }

    void genStmt(const ExpressionNode* n) {
        if (!n) return;
        switch (n->kind) {
        case NodeType::Block:
            genBlock(n);
            break;
        case NodeType::VarDecl:
            if (n->storage == VarStorage::Local) {
                allocLocal(n->textValue);
                if (n->rightSide) storeVar(n->textValue, genExpr(n->rightSide.get()));
            }
            break;
        case NodeType::Assign:
            storeVar(n->textValue, genExpr(n->rightSide.get()));
            break;
        case NodeType::ExprStmt:
            if (n->leftSide) genExpr(n->leftSide.get());
            break;
        case NodeType::Return:
            if (n->leftSide) {
                int v = genExpr(n->leftSide.get());
                emit({ (uint32_t)CommandID::MOV, (uint32_t)REG_RET, (uint32_t)v, REG_TO_REG });
            }
            emitEpilogue();
            break;
        case NodeType::If: {
            int elseLbl = newLabel();
            int endLbl = newLabel();
            genConditionJump(n->condition.get(), elseLbl);
            genStmt(n->thenBranch.get());
            int j = emit({ (uint32_t)CommandID::JMP, 0, 0, 0 });
            patchJump(j, endLbl);
            bindLabel(elseLbl);
            if (n->elseBranch) genStmt(n->elseBranch.get());
            bindLabel(endLbl);
            break;
        }
        case NodeType::While: {
            int start = newLabel();
            int end = newLabel();
            bindLabel(start);
            genConditionJump(n->condition.get(), end);
            genStmt(n->body.get());
            int j = emit({ (uint32_t)CommandID::JMP, 0, 0, 0 });
            patchJump(j, start);
            bindLabel(end);
            break;
        }
        case NodeType::DoWhile: {
            int start = newLabel();
            bindLabel(start);
            genStmt(n->body.get());
            if (n->condition && n->condition->kind == NodeType::LogicCmp) {
                int l = genExpr(n->condition->leftSide.get());
                int r = genExpr(n->condition->rightSide.get());
                emit({ (uint32_t)CommandID::CMP, (uint32_t)l, (uint32_t)r, 0 });
                RawInstruction br;
                br.opCode = (uint32_t)cmpBranch(n->condition->cmpOp);
                br.rs1 = (uint32_t)l;
                br.rs2 = (uint32_t)r;
                patchJump(emit(br), start);
            } else {
                int v = genExpr(n->condition.get());
                int z = emitLiteral(0);
                emit({ (uint32_t)CommandID::CMP, (uint32_t)v, (uint32_t)z, 0 });
                RawInstruction br;
                br.opCode = (uint32_t)CommandID::BR_NE;
                br.rs1 = (uint32_t)v;
                br.rs2 = (uint32_t)z;
                patchJump(emit(br), start);
            }
            break;
        }
        default:
            genExpr(n);
            break;
        }
    }

    void genBlock(const ExpressionNode* block) {
        for (const auto& s : block->nested)
            genStmt(s.get());
    }
};

} // namespace

CompileResult compileProgram(const ExpressionNode* program) {
    ExecFile exec;
    CodeGenerator gen(exec);
    return gen.compile(program);
}
