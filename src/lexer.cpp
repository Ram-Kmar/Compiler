// lexer.cpp
#include "lexer.h"
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string& source) : m_source(source) {}

std::vector<Token> Lexer::tokenize() {
    while (!is_at_end()) {
        skip_whitespace_and_comments();
        if (is_at_end()) break;

        char c = peek_char();
        if (isdigit(c)) {
            consume_number();
        } else if (isalpha(c) || c == '_') {
            std::string ident = consume_identifier();
            TokenType type = check_keyword(ident);
            if (type == TokenType::ident) add_token(type, ident);
            else add_token(type);
        } else if (c == '\'') {
            add_token(TokenType::char_lit, consume_char_literal());
        } else if (c == '"') {
            add_token(TokenType::string_lit, consume_string_literal());
        } else {
            switch (c) {
                case '+': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::plus_eq); }
                    else if (peek_char() == '+') { consume_char(); add_token(TokenType::plus_plus); }
                    else add_token(TokenType::plus); break;
                case '-': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::minus_eq); }
                    else if (peek_char() == '-') { consume_char(); add_token(TokenType::minus_minus); }
                    else if (peek_char() == '>') { consume_char(); add_token(TokenType::arrow); }
                    else add_token(TokenType::minus); break;
                case '*': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::star_eq); }
                    else add_token(TokenType::star); break;
                case '/': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::slash_eq); }
                    else add_token(TokenType::slash); break;
                case '%': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::percent_eq); }
                    else add_token(TokenType::percent); break;
                case '=': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::eq_eq); }
                    else add_token(TokenType::eq); break;
                case '!': consume_char(); 
                    if (peek_char() == '=') { consume_char(); add_token(TokenType::neq); }
                    else add_token(TokenType::bang); break;
                case '<': consume_char(); 
                    if (peek_char() == '<') { consume_char(); add_token(TokenType::lt_lt); }
                    else if (peek_char() == '=') { consume_char(); add_token(TokenType::lte); }
                    else add_token(TokenType::lt); break;
                case '>': consume_char(); 
                    if (peek_char() == '>') { consume_char(); add_token(TokenType::gt_gt); }
                    else if (peek_char() == '=') { consume_char(); add_token(TokenType::gte); }
                    else add_token(TokenType::gt); break;
                case '&': consume_char(); 
                    if (peek_char() == '&') { consume_char(); add_token(TokenType::amp_amp); }
                    else add_token(TokenType::amp); break;
                case '|': consume_char(); 
                    if (peek_char() == '|') { consume_char(); add_token(TokenType::pipe_pipe); }
                    else add_token(TokenType::pipe); break;
                case '^': consume_char(); add_token(TokenType::caret); break;
                case '~': consume_char(); add_token(TokenType::tilde); break;
                case '?': consume_char(); add_token(TokenType::question); break;
                case ':': consume_char(); add_token(TokenType::colon); break;
                case ';': consume_char(); add_token(TokenType::semi); break;
                case ',': consume_char(); add_token(TokenType::comma); break;
                case '.': consume_char(); add_token(TokenType::dot); break;
                case '(': consume_char(); add_token(TokenType::open_paren); break;
                case ')': consume_char(); add_token(TokenType::close_paren); break;
                case '{': consume_char(); add_token(TokenType::open_curly); break;
                case '}': consume_char(); add_token(TokenType::close_curly); break;
                case '[': consume_char(); add_token(TokenType::open_bracket); break;
                case ']': consume_char(); add_token(TokenType::close_bracket); break;
                default: consume_char(); break;
            }
        }
    }
    add_token(TokenType::eof);
    return m_tokens;
}

char Lexer::peek_char(int offset) const {
    if (m_index + offset >= m_source.size()) return '\0';
    return m_source[m_index + offset];
}

char Lexer::consume_char() {
    char c = peek_char();
    m_index++;
    if (c == '\n') { m_line++; m_col = 0; }
    else m_col++;
    return c;
}

bool Lexer::is_at_end() const { return m_index >= m_source.size(); }

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        char c = peek_char();
        if (isspace(c)) consume_char();
        else if (c == '/' && peek_char(1) == '/') {
            while (peek_char() != '\n' && !is_at_end()) consume_char();
        } else if (c == '/' && peek_char(1) == '*') {
            consume_char(); consume_char();
            while (!(peek_char() == '*' && peek_char(1) == '/') && !is_at_end()) consume_char();
            if (!is_at_end()) { consume_char(); consume_char(); }
        } else break;
    }
}

void Lexer::add_token(TokenType type) { 
    // DEBUG PRINT
    // std::cout << "[DEBUG Lexer] Added token type: " << (int)type << " at line " << m_line << std::endl;
    m_tokens.push_back({type, 0, m_line, m_col}); 
}
void Lexer::add_token(TokenType type, int value) { 
    std::cout << "[DEBUG Lexer] Added INT_LIT: " << value << std::endl;
    m_tokens.push_back({type, value, m_line, m_col}); 
}
void Lexer::add_token(TokenType type, double value) { 
    std::cout << "[DEBUG Lexer] Added FLOAT_LIT: " << value << std::endl;
    m_tokens.push_back({type, value, m_line, m_col}); 
}
void Lexer::add_token(TokenType type, char value) { 
    std::cout << "[DEBUG Lexer] Added CHAR_LIT: " << value << std::endl;
    m_tokens.push_back({type, value, m_line, m_col}); 
}
void Lexer::add_token(TokenType type, const std::string& value) { 
    std::cout << "[DEBUG Lexer] Added IDENT/STR: " << value << std::endl;
    m_tokens.push_back({type, value, m_line, m_col}); 
}

std::string Lexer::consume_identifier() {
    std::string ident;
    while (isalnum(peek_char()) || peek_char() == '_') ident += consume_char();
    return ident;
}

void Lexer::consume_number() {
    std::string res;
    bool is_float = false;
    while (isdigit(peek_char()) || peek_char() == '.') {
        if (peek_char() == '.') is_float = true;
        res += consume_char();
    }
    if (is_float) add_token(TokenType::float_lit, std::stod(res));
    else add_token(TokenType::int_lit, std::stoi(res));
}

char Lexer::consume_char_literal() {
    consume_char(); // '
    char c = consume_char();
    if (c == '\\') {
        char next = consume_char();
        if (next == 'n') c = '\n';
        else if (next == 't') c = '\t';
    }
    consume_char(); // '\'
    return c;
}

std::string Lexer::consume_string_literal() {
    consume_char(); // "
    std::string res;
    while (peek_char() != '"' && !is_at_end()) {
        char c = consume_char();
        if (c == '\\') {
            char next = consume_char();
            if (next == 'n') res += '\n';
            else if (next == 't') res += '\t';
            else res += next;
        } else res += c;
    }
    consume_char(); // "
    return res;
}

TokenType Lexer::check_keyword(const std::string& ident) {
    auto it = m_keywords.find(ident);
    return (it != m_keywords.end()) ? it->second : TokenType::ident;
}

std::vector<Token> tokenize(const std::string &src) { return Lexer(src).tokenize(); }

std::string token_to_string(const Token &token) {
    switch(token.type) {
        case TokenType::int_lit: return "INT_LIT";
        case TokenType::ident: return "IDENT";
        case TokenType::eof: return "EOF";
        case TokenType::_return: return "RETURN";
        case TokenType::_int: return "INT";
        case TokenType::semi: return "SEMI";
        default: return "TOKEN";
    }
}
