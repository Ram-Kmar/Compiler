#include "lexer.h"
#include <iostream>
#include <cctype>
#include <sstream>

std::string token_to_string(const Token& token) {
    switch (token.type) {
        case TokenType::_return: return "RETURN";
        case TokenType::_int:    return "INT";
        case TokenType::_if:     return "IF";
        case TokenType::_else:   return "ELSE";
        case TokenType::_while:  return "WHILE";
        case TokenType::semi:    return "SEMI";
        case TokenType::eq:      return "EQUALS";
        case TokenType::eq_eq:   return "EQ_EQ";
        case TokenType::neq:  return "NEQ";
        case TokenType::lt:      return "LT";
        case TokenType::gt:      return "GT";
        case TokenType::plus:    return "PLUS";
        case TokenType::minus:   return "MINUS";
        case TokenType::star:    return "STAR";
        case TokenType::slash:   return "SLASH";
        case TokenType::open_curly:  return "OPEN_CURLY";
        case TokenType::close_curly: return "CLOSE_CURLY";
        case TokenType::open_paren:  return "OPEN_PAREN";
        case TokenType::close_paren: return "CLOSE_PAREN";
        case TokenType::comma:       return "COMMA";
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
                } else if (buf == "if") {
                    tokens.push_back({TokenType::_if});
                } else if (buf == "else") {
                    tokens.push_back({TokenType::_else});
                } else if (buf == "while") {
                    tokens.push_back({TokenType::_while});
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
            if (i + 1 < src.length() && src[i + 1] == '=') {
                tokens.push_back({TokenType::eq_eq});
                i++; // Skip next char
            } else {
                tokens.push_back({TokenType::eq});
            }
        }
        else if (c == '!') {
            if (i + 1 < src.length() && src[i + 1] == '=') {
                tokens.push_back({TokenType::neq});
                i++; // Skip next char
            } else {
                std::cerr << "Error: Expected '=' after '!'" << std::endl;
                exit(1);
            }
        }
        else if (c == '<') {
            tokens.push_back({TokenType::lt});
        }
        else if (c == '>') {
            tokens.push_back({TokenType::gt});
        }
        else if (c == '+') {
            tokens.push_back({TokenType::plus});
        }
        else if (c == '-') {
            tokens.push_back({TokenType::minus});
        }
        else if (c == '*') {
            tokens.push_back({TokenType::star});
        }
        else if (c == '/') {
            tokens.push_back({TokenType::slash});
        }
        else if (c == '{') {
            tokens.push_back({TokenType::open_curly});
        }
        else if (c == '}') {
            tokens.push_back({TokenType::close_curly});
        }
        else if (c == '(') {
            tokens.push_back({TokenType::open_paren});
        }
        else if (c == ')') {
            tokens.push_back({TokenType::close_paren});
        }
        else if (c == ',') {
            tokens.push_back({TokenType::comma});
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
