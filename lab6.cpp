#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <iomanip>

using namespace std;

enum OpType { OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_HALT };
string labels[] = { "MOV", "ADD", "SUB", "MUL", "DIV", "HALT" };

struct Inst {
    OpType op;
    int r0, r1, r2;
    double val;
};

struct VM {
    double regs[1024] = { 0 };
};

enum LType { L_NUM, L_VAR, L_OP, L_LPR, L_RPR };
struct Token {
    LType type;
    string raw;
};

vector<Token> tokens;
int pos = 0;

void lex(string src) {
    tokens.clear();
    pos = 0;
    for (int i = 0; i < src.length(); ++i) {
        if (isspace(src[i])) continue;
        if (isdigit(src[i])) {
            string b;
            while (i < src.length() && (isdigit(src[i]) || src[i] == '.')) b += src[i++];
            tokens.push_back({ L_NUM, b }); i--;
        }
        else if (isalpha(src[i])) {
            string b;
            while (i < src.length() && isalnum(src[i])) b += src[i++];
            tokens.push_back({ L_VAR, b }); i--;
        }
        else {
            LType t = (src[i] == '(') ? L_LPR : (src[i] == ')') ? L_RPR : L_OP;
            tokens.push_back({ t, string(1, src[i]) });
        }
    }
}

enum NType { N_VAL, N_VAR, N_OP };
struct Node {
    NType type;
    string id;
    double v;
    Node* l = nullptr, * r = nullptr;
    Node(NType t, string s) : type(t), id(s), v(0) {}
    Node(double val) : type(N_VAL), id(""), v(val) {}
    ~Node() { delete l; delete r; }
};

Node* parseExpr();
Node* parseBase() {
    if (pos >= tokens.size()) return nullptr;
    Token t = tokens[pos++];
    if (t.type == L_NUM) return new Node(stod(t.raw));
    if (t.type == L_VAR) return new Node(N_VAR, t.raw);
    if (t.type == L_LPR) {
        Node* n = parseExpr();
        if (pos < tokens.size()) pos++;
        return n;
    }
    return nullptr;
}

Node* parseTerm() {
    Node* n = parseBase();
    while (pos < tokens.size() && (tokens[pos].raw == "*" || tokens[pos].raw == "/")) {
        string o = tokens[pos++].raw;
        Node* p = new Node(N_OP, o);
        p->l = n; p->r = parseBase();
        n = p;
    }
    return n;
}

Node* parseExpr() {
    Node* n = parseTerm();
    while (pos < tokens.size() && (tokens[pos].raw == "+" || tokens[pos].raw == "-")) {
        string o = tokens[pos++].raw;
        Node* p = new Node(N_OP, o);
        p->l = n; p->r = parseTerm();
        n = p;
    }
    return n;
}

int gen(Node* n, vector<Inst>& p, int& rc, map<string, int>& vm) {
    if (!n) return -1;
    if (n->type == N_VAL) {
        int r = rc++;
        p.push_back({ OP_MOV, r, -1, -1, n->v });
        return r;
    }
    if (n->type == N_VAR) {
        if (vm.find(n->id) == vm.end()) vm[n->id] = rc++;
        return vm[n->id];
    }
    int l = gen(n->l, p, rc, vm);
    int r = gen(n->r, p, rc, vm);
    int d = rc++;
    OpType o = (n->id == "+") ? OP_ADD : (n->id == "-") ? OP_SUB : (n->id == "*") ? OP_MUL : OP_DIV;
    p.push_back({ o, d, l, r, 0 });
    return d;
}

double run(const vector<Inst>& p, VM& m) {
    for (const auto& i : p) {
        switch (i.op) {
        case OP_MOV:  m.regs[i.r0] = i.val; break;
        case OP_ADD:  m.regs[i.r0] = m.regs[i.r1] + m.regs[i.r2]; break;
        case OP_SUB:  m.regs[i.r0] = m.regs[i.r1] - m.regs[i.r2]; break;
        case OP_MUL:  m.regs[i.r0] = m.regs[i.r1] * m.regs[i.r2]; break;
        case OP_DIV:  m.regs[i.r0] = (m.regs[i.r2] != 0) ? m.regs[i.r1] / m.regs[i.r2] : 0; break;
        case OP_HALT: return m.regs[i.r0];
        }
    }
    return 0;
}

int main() {
    string s;
    getline(cin, s);
    lex(s);
    Node* root = parseExpr();
    if (root) {
        VM m;
        vector<Inst> p;
        map<string, int> v;
        int rc = 0;
        int res = gen(root, p, rc, v);
        p.push_back({ OP_HALT, res, 0, 0, 0 });

        for (int i = 100; i <= 1000; i += 100) {
            double x = i / 100.0;
            if (v.count("x")) m.regs[v["x"]] = x;
            cout << "x=" << x << " -> " << run(p, m) << endl;
        }
    }
    delete root;
    return 0;
}