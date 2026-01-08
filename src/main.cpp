#include <iostream>
#include <fstream>
#include <sstream>
#include "lexer.h"
#include "parser.h"
#include "generation.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "compiler <input.hy>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        if (!input.is_open()) {
            std::cerr << "Could not open file: " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    // 1. Lexing
    std::cout << "--- Tokenization Step ---" << std::endl;
    std::vector<Token> tokens = tokenize(contents);
    for (const auto& token : tokens) {
        std::cout << token_to_string(token) << std::endl;
    }
    std::cout << "-------------------------" << std::endl;

    // 2. Parsing
    std::cout << "\n--- Parsing Step ---" << std::endl;
    Parser parser(tokens);
    std::unique_ptr<Program> program = parser.parse_program();

    if (!program) {
        std::cerr << "No parse tree generated" << std::endl;
        return EXIT_FAILURE;
    }
    program->print(); // Visualize the AST
    std::cout << "--------------------" << std::endl;

    // 3. Generation
    std::cout << "\n--- Generation Step ---" << std::endl;
    Generator generator(std::move(program));
    std::string assembly = generator.generate();
    std::cout << assembly << std::endl; // Print assembly to console
    std::cout << "-----------------------" << std::endl;

    {
        std::fstream file("out.s", std::ios::out);
        file << assembly;
    }

    return EXIT_SUCCESS;
}
