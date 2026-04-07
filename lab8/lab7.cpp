#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>
#include <memory>
#include <cstdint>

    using namespace std;

enum class ActionType : uint32_t { SET_VAL, GET_VAR, SUM, DIFF, MULT, RATIO, STOP };
string actionLabels[] = { "SET_VAL", "GET_VAR", "SUM", "DIFF", "MULT", "RATIO", "STOP" };

struct Command {
    uint32_t code : 4;
    uint32_t dest : 9;
    uint32_t arg1 : 9;
    uint32_t arg2 : 9;
};

static_assert(sizeof(Command) == 4);

enum class EntityType { CONSTANT, IDENTIFIER, CALCULATION };

struct TreeNode {
    EntityType kind;
    string rawText;
    double numeric;
    unique_ptr<TreeNode> leftSide;
    unique_ptr<TreeNode> rightSide;

    TreeNode(EntityType k, string t) : kind(k), rawText(t), numeric(0) {}
    TreeNode(double v) : kind(EntityType::CONSTANT), rawText(""), numeric(v) {}
};

enum Kind { DIGIT, WORD, SYMBOL, BR_OPEN, BR_CLOSE };
struct Unit {
    Kind type;
    string val;
};

vector<Unit> unitStream;
int unitPointer = 0;

void decompose(string raw) {
    unitStream.clear();
    unitPointer = 0;
    for (int i = 0; i < raw.length(); ++i) {
        if (isspace(raw[i])) continue;
        if (isdigit(raw[i])) {
            string s;
            while (i < raw.length() && (isdigit(raw[i]) || raw[i] == '.')) s += raw[i++];
            unitStream.push_back({ DIGIT, s }); i--;
        }
        else if (isalpha(raw[i])) {
            string s;
            while (i < raw.length() && isalnum(raw[i])) s += raw[i++];
            unitStream.push_back({ WORD, s }); i--;
        }
        else {
            Kind k = (raw[i] == '(') ? BR_OPEN : (raw[i] == ')') ? BR_CLOSE : SYMBOL;
            unitStream.push_back({ k, string(1, raw[i]) });
        }
    }
}


unique_ptr<TreeNode> buildExpr();

unique_ptr<TreeNode> buildLeaf() {
    if (unitPointer >= unitStream.size()) return nullptr;
    Unit u = unitStream[unitPointer++];
    if (u.type == DIGIT) return make_unique<TreeNode>(stod(u.val));
    if (u.type == WORD) return make_unique<TreeNode>(EntityType::IDENTIFIER, u.val);
    if (u.type == BR_OPEN) {
        auto sub = buildExpr();
        if (unitPointer < unitStream.size()) unitPointer++;
        return sub;
    }
    return nullptr;
}

unique_ptr<TreeNode> buildTerm() {
    auto node = buildLeaf();
    while (unitPointer < unitStream.size() && (unitStream[unitPointer].val == "*" || unitStream[unitPointer].val == "/")) {
        string op = unitStream[unitPointer++].val;
        auto p = make_unique<TreeNode>(EntityType::CALCULATION, op);
        p->leftSide = std::move(node);
        p->rightSide = buildLeaf();
        node = std::move(p);
    }
    return node;
}

unique_ptr<TreeNode> buildExpr() {
    auto node = buildTerm();
    while (unitPointer < unitStream.size() && (unitStream[unitPointer].val == "+" || unitStream[unitPointer].val == "-")) {
        string op = unitStream[unitPointer++].val;
        auto p = make_unique<TreeNode>(EntityType::CALCULATION, op);
        p->leftSide = std::move(node);
        p->rightSide = buildTerm();
        node = std::move(p);
    }
    return node;
}

int translate(const TreeNode* node, vector<Command>& rom, vector<double>& constants, int& regIdx, map<string, int>& varTable) {
    if (!node) return -1;
    if (node->kind == EntityType::CONSTANT) {
        int r = regIdx++;
        uint32_t cPos = constants.size();
        constants.push_back(node->numeric);
        rom.push_back({ (uint32_t)ActionType::SET_VAL, (uint32_t)r, cPos, 0 });
        return r;
    }
    if (node->kind == EntityType::IDENTIFIER) {
        int r = regIdx++;
        if (varTable.find(node->rawText) == varTable.end()) {
            int id = varTable.size();
            varTable[node->rawText] = id;
        }
        rom.push_back({ (uint32_t)ActionType::GET_VAR, (uint32_t)r, (uint32_t)varTable[node->rawText], 0 });
        return r;
    }
    int l = translate(node->leftSide.get(), rom, constants, regIdx, varTable);
    int r = translate(node->rightSide.get(), rom, constants, regIdx, varTable);
    int target = regIdx++;
    ActionType type = (node->rawText == "+") ? ActionType::SUM : (node->rawText == "-") ? ActionType::DIFF :
        (node->rawText == "*") ? ActionType::MULT : ActionType::RATIO;
    rom.push_back({ (uint32_t)type, (uint32_t)target, (uint32_t)l, (uint32_t)r });
    return target;
}

double engine(const vector<Command>& rom, const vector<double>& pool, const vector<double>& inputs, double* stack) {
    for (const auto& c : rom) {
        switch ((ActionType)c.code) {
        case ActionType::SET_VAL: stack[c.dest] = pool[c.arg1]; break;
        case ActionType::GET_VAR: stack[c.dest] = inputs[c.arg1]; break;
        case ActionType::SUM:     stack[c.dest] = stack[c.arg1] + stack[c.arg2]; break;
        case ActionType::DIFF:    stack[c.dest] = stack[c.arg1] - stack[c.arg2]; break;
        case ActionType::MULT:    stack[c.dest] = stack[c.arg1] * stack[c.arg2]; break;
        case ActionType::RATIO:   stack[c.dest] = (stack[c.arg2] != 0) ? stack[c.arg1] / stack[c.arg2] : 0; break;
        case ActionType::STOP:    return stack[c.dest];
        }
    }
    return 0;
}

void showBytecode(const vector<Command>& rom, const vector<double>& pool, const map<string, int>& varTable) {
    map<int, string> idToName;
    for (auto const& [name, id] : varTable) idToName[id] = name;
    for (int i = 0; i < rom.size(); ++i) {
        const auto& c = rom[i];
        cout << "[" << i << "] " << left << setw(10) << actionLabels[c.code] << " r" << c.dest << ", ";
        if ((ActionType)c.code == ActionType::SET_VAL) cout << pool[c.arg1];
        else if ((ActionType)c.code == ActionType::GET_VAR) cout << idToName[c.arg1];
        else if ((ActionType)c.code == ActionType::STOP) cout << "exit";
        else cout << "r" << c.arg1 << ", r" << c.arg2;
        cout << endl;
    }
}

int main() {
    string input;
    cout << "Formula: ";
    getline(cin, input);
    decompose(input);
    auto root = buildExpr();
    if (root) {
        vector<Command> rom;
        vector<double> pool;
        map<string, int> varTable;
        int regCounter = 0;
        int resReg = translate(root.get(), rom, pool, regCounter, varTable);
        rom.push_back({ (uint32_t)ActionType::STOP, (uint32_t)resReg, 0, 0 });

        showBytecode(rom, pool, varTable);

        vector<double> inputs(varTable.size(), 0);
        double stack[1024] = { 0 };
        cout << fixed << setprecision(2);
        for (int i = 100; i <= 2000; i += 200) {
            double x = i / 100.0;
            if (varTable.count("x")) inputs[varTable["x"]] = x;
            double res = engine(rom, pool, inputs, stack);
            cout << "x = " << x << " -> Res: " << res << endl;
        }
    }
    return 0;
}
