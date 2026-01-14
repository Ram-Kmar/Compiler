#include "../src/lexer.h"
#include "../src/parser.h"
#include <iostream>
#include <vector>

int main() {
    // A sample C program using your new features
    std::string source_code = R"(
        struct Point {
            int x;
            int y;
        };

        int main() {
            struct Point p;
            p.x = 10;
            p.y = 20;

            char* msg = "Hello, World!";
            int count = 0;

            do {
                count += 1;
                if (count > 5) {
                    break;
                }
            } while (count < 10);

            switch (count) {
                case 1: 
                    count = 100;
                case 6:
                    count = 200;
                default:
                    count = 0;
            }

            return count;
        }
    )";

    std::cout << "=== SOURCE CODE ===" << std::endl;
    std::cout << source_code << std::endl;
    std::cout << "===================\n" << std::endl;

    // 1. Test Lexer
    std::cout << "=== LEXER OUTPUT ===" << std::endl;
    Lexer lexer(source_code);
    std::vector<Token> tokens = lexer.tokenize();
    
    for (const auto& token : tokens) {
        std::cout << token_to_string(token) << " ";
        if (token.type == TokenType::semi || token.type == TokenType::open_curly || token.type == TokenType::close_curly) {
            std::cout << "\n";
        }
    }
    std::cout << "\n====================\n" << std::endl;

    // 2. Test Parser
    std::cout << "=== PARSER OUTPUT (AST) ===" << std::endl;
    Parser parser(tokens);
    std::unique_ptr<Program> program = parser.parse_program();

    if (program) {
        program->print();
    } else {
        std::cerr << "Parsing failed!" << std::endl;
    }
    std::cout << "===========================" << std::endl;

    return 0;
}
