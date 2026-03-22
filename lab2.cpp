#include <iostream>
#include <string>
#include <map>
#include <cctype>
#include <stdexcept>
#include <iomanip>

class Parser {
public:
    Parser(const std::string& expr, const std::map<char, double>& vars)
        : expr(expr), vars(vars), pos(0) {
    }

    double parse() {
        return parseExpression();
    }

private:
    std::string expr;               
    std::map<char, double> vars;     
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
            else {
                if (right == 0) throw std::runtime_error("Division by zero");
                left /= right;
            }
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
        else if (std::isalpha(c)) {          
            char varName = c;
            pos++;
            auto it = vars.find(varName);
            if (it == vars.end()) {
                throw std::runtime_error("Unknown variable: " + std::string(1, varName));
            }
            return it->second;
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
    std::cout << "Enter expression (use 'x' as variable): ";
    std::string expr;
    std::getline(std::cin, expr);

    std::cout << "Evaluating for x from 1 to 100 with step 0.01:\n";
    std::cout << std::fixed << std::setprecision(6);

    const double start = 1.0;
    const double end = 100.0;
    const double step = 0.01;
    int numSteps = static_cast<int>((end - start) / step + 0.5) + 1; 

    for (int i = 0; i < numSteps; ++i) {
        double x = start + i * step;                
        std::map<char, double> vars;
        vars['x'] = x;                          

        try {
            Parser parser(expr, vars);
            double result = parser.parse();
            std::cout << "x = " << x << ", result = " << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cerr << "Error at x = " << x << ": " << e.what() << std::endl;
            break;                                  
        }
    }

    return 0;
}