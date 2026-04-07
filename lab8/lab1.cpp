#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>

class Parser {
public:
    Parser(const std::string& expr) : expr(expr), pos(0) {}

    double parse() {
        return parseExpression();
    }

private:
    std::string expr;
    size_t pos;

    void skipWhitespace() {
        while (pos < expr.length() && std::isspace(expr[pos])) pos++;
    }

    double parseExpression() {
        double left = parseTerm();
        skipWhitespace();
        while (pos < expr.length() && (expr[pos] == '+' || expr[pos] == '-')) {
            char op = expr[pos];
            pos++;
            double right = parseTerm();
            if (op == '+') left += right;
            else left -= right;
            skipWhitespace();
        }
        return left;
    }

    double parseTerm() {
        double left = parseFactor();
        skipWhitespace();
        while (pos < expr.length() && (expr[pos] == '*' || expr[pos] == '/')) {
            char op = expr[pos];
            pos++;
            double right = parseFactor();
            if (op == '*') left *= right;
            else left /= right; 
            skipWhitespace();
        }
        return left;
    }

    double parseFactor() {
        skipWhitespace();
        if (pos >= expr.length()) throw std::runtime_error("Unexpected end of expression");

        char c = expr[pos];
        if (c == '(') {
            pos++; 
            double value = parseExpression();
            skipWhitespace();
            if (pos >= expr.length() || expr[pos] != ')')
                throw std::runtime_error("Missing closing parenthesis");
            pos++; 
            return value;
        }
        else if (c == '-') {
            
            pos++;
            return -parseFactor();
        }
        else if (std::isdigit(c) || c == '.') {
            
            size_t start = pos;
            bool hasDecimal = false;
            while (pos < expr.length() && (std::isdigit(expr[pos]) || expr[pos] == '.')) {
                if (expr[pos] == '.') {
                    if (hasDecimal) throw std::runtime_error("Invalid number format");
                    hasDecimal = true;
                }
                pos++;
            }
            std::string numStr = expr.substr(start, pos - start);
            return std::stod(numStr);
        }
        else {
            throw std::runtime_error("Unexpected character");
        }
    }
};

int main() {
    std::cout << "Enter expression: ";
    std::string expr;
    std::getline(std::cin, expr);

    try {
        Parser parser(expr);
        double result = parser.parse();
        std::cout << "Result: " << result << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}