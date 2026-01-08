#pragma once
#include <vector>
#include <string>
#include <optional>
#include <variant>

enum class TokenType {
    _return,
    _int,
    int_lit,
    semi,
    ident,
    eq
};

struct Token {
    TokenType type;
    std::variant<std::monostate, int, std::string> value;
};

std::vector<Token> tokenize(const std::string& src);
std::string token_to_string(const Token& token);
