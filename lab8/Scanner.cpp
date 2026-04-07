#include "Scanner.h"
#include <cctype>

std::vector<TokenUnit> tokenStream;
int streamPos = 0;

void runScanner(const std::string& input) {
    tokenStream.clear();
    streamPos = 0;
    int i = 0;
    int len = input.length();

    while (i < len) {
        if (isspace(input[i])) {
            i++;
            continue;
        }

        
        if (isdigit(input[i])) {
            std::string val = "";
            while (i < len && (isdigit(input[i]) || input[i] == '.')) {
                val += input[i++];
            }
            tokenStream.push_back({ TokenKind::NUM, val });
        }
       
        else if (isalpha(input[i])) {
            std::string word = "";
            while (i < len && isalnum(input[i])) {
                word += input[i++];
            }

            if (word == "if") tokenStream.push_back({ TokenKind::K_IF, word });
            else if (word == "else") tokenStream.push_back({ TokenKind::K_ELSE, word });
            else if (word == "return") tokenStream.push_back({ TokenKind::K_RET, word });
            else tokenStream.push_back({ TokenKind::IDENT, word });
        }
     
        else {
            std::string s(1, input[i]);
            if (s == "+") tokenStream.push_back({ TokenKind::OP, s });
            else if (s == "-") tokenStream.push_back({ TokenKind::OP, s });
            else if (s == "*") tokenStream.push_back({ TokenKind::OP, s });
            else if (s == "/") tokenStream.push_back({ TokenKind::OP, s });
            else if (s == "(") tokenStream.push_back({ TokenKind::L_PAR, s });
            else if (s == ")") tokenStream.push_back({ TokenKind::R_PAR, s });
            else if (s == "{") tokenStream.push_back({ TokenKind::L_BRACE, s });
            else if (s == "}") tokenStream.push_back({ TokenKind::R_BRACE, s });
            else if (s == ";") tokenStream.push_back({ TokenKind::SEMI, s });
            i++;
        }
    }
}

TokenUnit& activeToken() {
    return tokenStream[streamPos];
}

bool isEndOfStream() {
    return streamPos >= (int)tokenStream.size();
}

TokenUnit pullToken() {
    return tokenStream[streamPos++];
}