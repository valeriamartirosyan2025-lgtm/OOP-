#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <stack>
#include <cmath>
#include <iomanip>
#include <stdexcept>

enum class TokenType {
    NUMBER,
    VARIABLE,      
    OPERATOR,     
    LEFT_PAREN,
    RIGHT_PAREN,
    END
};

struct Token {
    TokenType type;
    std::string value;      
    double numberValue;     
};

class Lexer {
public:
    Lexer(const std::string& input) : input(input), pos(0) {}

 
    Token nextToken() {
        skipWhitespace();
        if (pos >= input.length()) {
            return Token{ TokenType::END, "", 0.0 };
        }

        char c = input[pos];
        if (c == '(') {
            pos++;
            return Token{ TokenType::LEFT_PAREN, "(", 0.0 };
        }
        if (c == ')') {
            pos++;
            return Token{ TokenType::RIGHT_PAREN, ")", 0.0 };
        }
        if (c == '+' || c == '-' || c == '*' || c == '/') {
            pos++;
            return Token{ TokenType::OPERATOR, std::string(1, c), 0.0 };
        }
        if (std::isalpha(c)) {
            
            if (c == 'x') {
                pos++;
                return Token{ TokenType::VARIABLE, "x", 0.0 };
            }
            else {
                throw std::runtime_error("Unknown variable: only 'x' is allowed");
            }
        }
        if (std::isdigit(c) || c == '.') {
            size_t start = pos;
            bool hasDecimal = false;
            while (pos < input.length() && (std::isdigit(input[pos]) || input[pos] == '.')) {
                if (input[pos] == '.') {
                    if (hasDecimal) throw std::runtime_error("Invalid number format");
                    hasDecimal = true;
                }
                pos++;
            }
            std::string numStr = input.substr(start, pos - start);
            double val = std::stod(numStr);
            return Token{ TokenType::NUMBER, numStr, val };
        }
        throw std::runtime_error(std::string("Unexpected character: ") + c);
    }

private:
    std::string input;
    size_t pos;

    void skipWhitespace() {
        while (pos < input.length() && std::isspace(input[pos])) pos++;
    }
};

std::vector<Token> infixToRPN(const std::string& expr) {
    Lexer lexer(expr);
    std::vector<Token> output;          
    std::stack<Token> operators;       

    
    auto precedence = [](char op) -> int {
        if (op == '+' || op == '-') return 1;
        if (op == '*' || op == '/') return 2;
        return 0;
        };

    Token token = lexer.nextToken();
    while (token.type != TokenType::END) {
        switch (token.type) {
        case TokenType::NUMBER:
        case TokenType::VARIABLE:
            output.push_back(token);
            break;

        case TokenType::LEFT_PAREN:
            operators.push(token);
            break;

        case TokenType::RIGHT_PAREN:
            
            while (!operators.empty() && operators.top().type != TokenType::LEFT_PAREN) {
                output.push_back(operators.top());
                operators.pop();
            }
            if (operators.empty()) {
                throw std::runtime_error("Mismatched parentheses");
            }
            operators.pop(); 
            break;

        case TokenType::OPERATOR:
        {
            char op = token.value[0];
            while (!operators.empty() && operators.top().type == TokenType::OPERATOR) {
                char topOp = operators.top().value[0];
                if (precedence(topOp) >= precedence(op)) {
                    output.push_back(operators.top());
                    operators.pop();
                }
                else {
                    break;
                }
            }
            operators.push(token);
        }
        break;

        default:
            throw std::runtime_error("Unexpected token type");
        }
        token = lexer.nextToken();
    }

    
    while (!operators.empty()) {
        if (operators.top().type == TokenType::LEFT_PAREN) {
            throw std::runtime_error("Mismatched parentheses");
        }
        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}


double evaluateRPN(const std::vector<Token>& rpn, double x) {
    std::stack<double> st;

    for (const Token& tok : rpn) {
        if (tok.type == TokenType::NUMBER) {
            st.push(tok.numberValue);
        }
        else if (tok.type == TokenType::VARIABLE) {
            st.push(x);
        }
        else if (tok.type == TokenType::OPERATOR) {
            if (st.size() < 2) throw std::runtime_error("Invalid expression");
            double right = st.top(); st.pop();
            double left = st.top(); st.pop();
            char op = tok.value[0];
            switch (op) {
            case '+': st.push(left + right); break;
            case '-': st.push(left - right); break;
            case '*': st.push(left * right); break;
            case '/':
                if (right == 0) throw std::runtime_error("Division by zero");
                st.push(left / right); break;
            default: throw std::runtime_error("Unknown operator");
            }
        }
    }
    if (st.size() != 1) throw std::runtime_error("Invalid expression");
    return st.top();
}


int main() {
    std::cout << "Enter expression (use 'x' as variable): ";
    std::string expr;
    std::getline(std::cin, expr);

    try {
        
        std::vector<Token> rpn = infixToRPN(expr);

        std::cout << "Evaluating for x from 1 to 100 with step 0.01:\n";
        std::cout << std::fixed << std::setprecision(6);

        const double start = 1.0;
        const double end = 100.0;
        const double step = 0.01;
        int numSteps = static_cast<int>((end - start) / step + 0.5) + 1; 

        for (int i = 0; i < numSteps; ++i) {
            double x = start + i * step;
            try {
                double result = evaluateRPN(rpn, x);
                std::cout << "x = " << x << ", result = " << result << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Error at x = " << x << ": " << e.what() << std::endl;
                break;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
