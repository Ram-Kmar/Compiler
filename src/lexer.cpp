#include "lexer.h"
#include <iostream>
#include <cctype>
#include <sstream>

std::string token_to_string(const Token& token) {
    switch (token.type) {
        case TokenType::_return: return "RETURN";
        case TokenType::_int:    return "INT";
        case TokenType::semi:    return "SEMI";
        case TokenType::eq:      return "EQUALS";
        case TokenType::int_lit: 
            return "INT_LIT(" + std::to_string(std::get<int>(token.value)) + ")";
        case TokenType::ident:   
            return "IDENT(" + std::get<std::string>(token.value) + ")";
        default: return "UNKNOWN";
    }
}

std::vector<Token> tokenize(const std::string& src) {
    std::vector<Token> tokens;
    std::string buf;

    for (int i = 0; i < src.length(); i++) {
        char c = src[i];

        if (std::isalpha(c)) {
            buf.push_back(c);
            if (i + 1 >= src.length() || !std::isalnum(src[i + 1])) {
                if (buf == "return") {
                    tokens.push_back({TokenType::_return});
                } else if (buf == "int") {
                    tokens.push_back({TokenType::_int});
                } else {
                    tokens.push_back({TokenType::ident, buf});
                }
                buf.clear();
            }
        }
        else if (std::isdigit(c)) {
            buf.push_back(c);
            if (i + 1 >= src.length() || !std::isdigit(src[i + 1])) {
                tokens.push_back({TokenType::int_lit, std::stoi(buf)});
                buf.clear();
            }
        }
        else if (c == ';') {
            tokens.push_back({TokenType::semi});
        }
        else if (c == '=') {
            tokens.push_back({TokenType::eq});
        }
        else if (std::isspace(c)) {
            continue;
        }
        else {
            std::cerr << "Error: Unknown character '" << c << "'" << std::endl;
            exit(1);
        }
    }
    return tokens;
}