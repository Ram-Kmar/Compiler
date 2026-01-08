#pragma once
#include <vector>
#include <string>
#include <optional>
#include <variant>

enum class TokenType {
    _return,
    _int,
    _if,
    _else,
    _while,
    int_lit,
    semi,
    ident,
    eq,
    plus,
    minus,
    star,
    slash,
    eq_eq,
    neq,
    lt,
    gt,
    open_curly,
    close_curly,
    open_paren,
    close_paren,
    comma
};

struct Token {
    TokenType type;
    std::variant<std::monostate, int, std::string> value;
};

std::vector<Token> tokenize(const std::string& src);
std::string token_to_string(const Token& token);
