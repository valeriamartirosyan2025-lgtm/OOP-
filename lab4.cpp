#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <memory>
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


struct Node {
    virtual ~Node() = default;
    virtual double evaluate(double x) const = 0;
};

struct NumberNode : Node {
    double value;
    NumberNode(double val) : value(val) {}
    double evaluate(double ) const override { return value; }
};


struct VariableNode : Node {
    double evaluate(double x) const override { return x; }
};


struct BinaryOpNode : Node {
    char op;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    BinaryOpNode(char operation, std::unique_ptr<Node> lhs, std::unique_ptr<Node> rhs)
        : op(operation), left(std::move(lhs)), right(std::move(rhs)) {
    }

    double evaluate(double x) const override {
        double l = left->evaluate(x);
        double r = right->evaluate(x);
        switch (op) {
        case '+': return l + r;
        case '-': return l - r;
        case '*': return l * r;
        case '/':
            if (r == 0) throw std::runtime_error("Division by zero");
            return l / r;
        default: throw std::runtime_error("Unknown operator");
        }
    }
};


class Parser {
public:
    Parser(Lexer& lex) : lex(lex) {
        currentToken = lex.nextToken();
    }

    std::unique_ptr<Node> parse() {
        return parseExpression();
    }

private:
    Lexer& lex;
    Token currentToken;

    void advance() {
        currentToken = lex.nextToken();
    }

    bool check(TokenType type) const {
        return currentToken.type == type;
    }

    void expect(TokenType type, const std::string& message) {
        if (currentToken.type != type) {
            throw std::runtime_error(message);
        }
        advance();
    }

  
    std::unique_ptr<Node> parseExpression() {
        auto left = parseTerm();
        while (check(TokenType::OPERATOR) &&
            (currentToken.value == "+" || currentToken.value == "-")) {
            char op = currentToken.value[0];
            advance();  
            auto right = parseTerm();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Node> parseTerm() {
        auto left = parseFactor();
        while (check(TokenType::OPERATOR) &&
            (currentToken.value == "*" || currentToken.value == "/")) {
            char op = currentToken.value[0];
            advance();
            auto right = parseFactor();
            left = std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right));
        }
        return left;
    }

    std::unique_ptr<Node> parseFactor() {
        if (check(TokenType::NUMBER)) {
            double val = currentToken.numberValue;
            advance();
            return std::make_unique<NumberNode>(val);
        }
        else if (check(TokenType::VARIABLE)) {
            advance(); 
            return std::make_unique<VariableNode>();
        }
        else if (check(TokenType::LEFT_PAREN)) {
            advance();  
            auto expr = parseExpression();
            expect(TokenType::RIGHT_PAREN, "Missing closing parenthesis");
            return expr;
        }
        else if (check(TokenType::OPERATOR) && currentToken.value == "-") {
            
            advance();
            auto factor = parseFactor();
            
            auto zero = std::make_unique<NumberNode>(0.0);
            return std::make_unique<BinaryOpNode>('-', std::move(zero), std::move(factor));
        }
        else {
            throw std::runtime_error("Unexpected token in factor");
        }
    }
};


int main() {
    std::cout << "Enter expression (use 'x' as variable): ";
    std::string expr;
    std::getline(std::cin, expr);

    try {
        Lexer lexer(expr);
        Parser parser(lexer);
        std::unique_ptr<Node> ast = parser.parse();

        

        std::cout << "Evaluating for x from 1 to 100 with step 0.01:\n";
        std::cout << std::fixed << std::setprecision(6);

        const double start = 1.0;
        const double end = 100.0;
        const double step = 0.01;
        int numSteps = static_cast<int>((end - start) / step + 0.5) + 1; 

        for (int i = 0; i < numSteps; ++i) {
            double x = start + i * step;
            try {
                double result = ast->evaluate(x);
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