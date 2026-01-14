// lexer.h
#pragma once
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>

enum class TokenType {
    // Literals
    int_lit,
    char_lit,
    float_lit,
    string_lit,
    _true,
    _false,

    // Identifiers
    ident,

    // Keywords
    _int, _char, _float, _double, _bool, _void,
    _struct, _union, _enum, _typedef,
    _return, _if, _else, _while, _do, _for, _break, _continue,
    _switch, _case, _default, _sizeof, _static, _extern, _const,

    // Operators
    plus, minus, star, slash, percent,
    plus_eq, minus_eq, star_eq, slash_eq, percent_eq,
    plus_plus, minus_minus,
    arrow, dot,
    eq, eq_eq, neq,
    lt, lte, gt, gte,
    amp, amp_amp, pipe, pipe_pipe, caret, tilde, bang,
    lt_lt, gt_gt,
    question, colon,
    comma, semi,
    open_paren, close_paren,
    open_bracket, close_bracket,
    open_curly, close_curly,

    eof
};

struct Token {
    TokenType type;
    std::variant<int, char, double, std::string> value;
    int line;
    int col;
};

class Lexer {
public:
    Lexer(const std::string& source);
    std::vector<Token> tokenize();

private:
    const std::string m_source;
    size_t m_index = 0;
    int m_line = 1;
    int m_col = 0;
    std::vector<Token> m_tokens;

    char peek_char(int offset = 0) const;
    char consume_char();
    bool is_at_end() const;
    void skip_whitespace_and_comments();

    void add_token(TokenType type);
    void add_token(TokenType type, int value);
    void add_token(TokenType type, double value);
    void add_token(TokenType type, char value);
    void add_token(TokenType type, const std::string& value);

    std::string consume_identifier();
    void consume_number();
    char consume_char_literal();
    std::string consume_string_literal();

    TokenType check_keyword(const std::string& ident);

    const std::unordered_map<std::string, TokenType> m_keywords = {
        {"int", TokenType::_int}, {"char", TokenType::_char}, {"float", TokenType::_float},
        {"double", TokenType::_double}, {"bool", TokenType::_bool}, {"void", TokenType::_void},
        {"struct", TokenType::_struct}, {"union", TokenType::_union}, {"enum", TokenType::_enum},
        {"typedef", TokenType::_typedef}, {"return", TokenType::_return}, {"if", TokenType::_if},
        {"else", TokenType::_else}, {"while", TokenType::_while}, {"do", TokenType::_do},
        {"for", TokenType::_for}, {"break", TokenType::_break}, {"continue", TokenType::_continue},
        {"switch", TokenType::_switch}, {"case", TokenType::_case}, {"default", TokenType::_default},
        {"sizeof", TokenType::_sizeof}, {"static", TokenType::_static}, {"extern", TokenType::_extern},
        {"const", TokenType::_const}, {"true", TokenType::_true}, {"false", TokenType::_false}
    };
};

std::vector<Token> tokenize(const std::string &src);
std::string token_to_string(const Token &token);