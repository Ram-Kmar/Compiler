#pragma once
#include <optional>
#include <string>
#include <variant>
#include <vector>

enum class TokenType {
  _return,
  _int,
  _bool,
  _true,
  _false,
  _if,
  _else,
  _while,
  _for,
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
  amp,
  amp_amp,
  pipe_pipe,
  bang,
  open_curly,
  close_curly,
  open_paren,
  close_paren,
  comma,
  open_bracket,
  close_bracket
};

struct Token {
  TokenType type;
  std::variant<std::monostate, int, std::string> value;
  int line;
  int col;
};

std::vector<Token> tokenize(const std::string &src);
std::string token_to_string(const Token &token);
