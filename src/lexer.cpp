#include "lexer.h"
#include <cctype>
#include <iostream>
#include <sstream>

std::string token_to_string(const Token &token) {
  std::string s;
  switch (token.type) {
  case TokenType::_return:
    s = "RETURN";
    break;
  case TokenType::_int:
    s = "INT";
    break;
  case TokenType::_bool:
    s = "BOOL";
    break;
  case TokenType::_true:
    s = "TRUE";
    break;
  case TokenType::_false:
    s = "FALSE";
    break;
  case TokenType::_if:
    s = "IF";
    break;
  case TokenType::_else:
    s = "ELSE";
    break;
  case TokenType::_while:
    s = "WHILE";
    break;
  case TokenType::_for:
    s = "FOR";
    break;
  case TokenType::semi:
    s = "SEMI";
    break;
  case TokenType::eq:
    s = "EQUALS";
    break;
  case TokenType::eq_eq:
    s = "EQ_EQ";
    break;
  case TokenType::neq:
    s = "NEQ";
    break;
  case TokenType::lt:
    s = "LT";
    break;
  case TokenType::gt:
    s = "GT";
    break;
  case TokenType::amp:
    s = "AMP";
    break;
  case TokenType::amp_amp:
    s = "AMP_AMP";
    break;
  case TokenType::pipe_pipe:
    s = "PIPE_PIPE";
    break;
  case TokenType::bang:
    s = "BANG";
    break;
  case TokenType::plus:
    s = "PLUS";
    break;
  case TokenType::minus:
    s = "MINUS";
    break;
  case TokenType::star:
    s = "STAR";
    break;
  case TokenType::slash:
    s = "SLASH";
    break;
  case TokenType::open_curly:
    s = "OPEN_CURLY";
    break;
  case TokenType::close_curly:
    s = "CLOSE_CURLY";
    break;
  case TokenType::open_paren:
    s = "OPEN_PAREN";
    break;
  case TokenType::close_paren:
    s = "CLOSE_PAREN";
    break;
  case TokenType::comma:
    s = "COMMA";
    break;
  case TokenType::open_bracket:
    s = "OPEN_BRACKET";
    break;
  case TokenType::close_bracket:
    s = "CLOSE_BRACKET";
    break;
  case TokenType::int_lit:
    s = "INT_LIT(" + std::to_string(std::get<int>(token.value)) + ")";
    break;
  case TokenType::ident:
    s = "IDENT(" + std::get<std::string>(token.value) + ")";
    break;
  default:
    s = "UNKNOWN";
    break;
  }
  return s + " (" + std::to_string(token.line) + ":" +
         std::to_string(token.col) + ")";
}

std::vector<Token> tokenize(const std::string &src) {
  std::vector<Token> tokens;
  std::string buf;
  int line = 1;
  int col = 1;

  for (int i = 0; i < src.length(); i++) {
    char c = src[i];
    int start_col = col;

    if (std::isalpha(c)) {
      buf.push_back(c);
      while (i + 1 < src.length() && std::isalnum(src[i + 1])) {
        buf.push_back(src[++i]);
        col++;
      }

      TokenType type;
      if (buf == "return")
        type = TokenType::_return;
      else if (buf == "int")
        type = TokenType::_int;
      else if (buf == "bool")
        type = TokenType::_bool;
      else if (buf == "true")
        type = TokenType::_true;
      else if (buf == "false")
        type = TokenType::_false;
      else if (buf == "if")
        type = TokenType::_if;
      else if (buf == "else")
        type = TokenType::_else;
      else if (buf == "while")
        type = TokenType::_while;
      else if (buf == "for")
        type = TokenType::_for;
      else {
        tokens.push_back({TokenType::ident, buf, line, start_col});
        buf.clear();
        continue;
      }
      tokens.push_back({type, std::monostate{}, line, start_col});
      buf.clear();
    } else if (std::isdigit(c)) {
      buf.push_back(c);
      col++;
      while (i + 1 < src.length() && std::isdigit(src[i + 1])) {
        buf.push_back(src[++i]);
        col++;
      }
      tokens.push_back({TokenType::int_lit, std::stoi(buf), line, start_col});
      buf.clear();
    } else if (c == ';') {
      tokens.push_back({TokenType::semi, std::monostate{}, line, col++});
    } else if (c == '=') {
      if (i + 1 < src.length() && src[i + 1] == '=') {
        tokens.push_back({TokenType::eq_eq, std::monostate{}, line, col});
        i++;
        col += 2;
      } else {
        tokens.push_back({TokenType::eq, std::monostate{}, line, col++});
      }
    } else if (c == '!') {
      if (i + 1 < src.length() && src[i + 1] == '=') {
        tokens.push_back({TokenType::neq, std::monostate{}, line, col});
        i++;
        col += 2;
      } else {
        tokens.push_back({TokenType::bang, std::monostate{}, line, col++});
      }
    } else if (c == '&') {
      if (i + 1 < src.length() && src[i + 1] == '&') {
        tokens.push_back({TokenType::amp_amp, std::monostate{}, line, col});
        i++;
        col += 2;
      } else {
        tokens.push_back({TokenType::amp, std::monostate{}, line, col++});
      }
    } else if (c == '|') {
      if (i + 1 < src.length() && src[i + 1] == '|') {
        tokens.push_back({TokenType::pipe_pipe, std::monostate{}, line, col});
        i++;
        col += 2;
      } else {
        std::cerr << "Error: Expected '|' after '|' at " << line << ":" << col
                  << std::endl;
        exit(1);
      }
    } else if (c == '<') {
      tokens.push_back({TokenType::lt, std::monostate{}, line, col++});
    } else if (c == '>') {
      tokens.push_back({TokenType::gt, std::monostate{}, line, col++});
    } else if (c == '+') {
      tokens.push_back({TokenType::plus, std::monostate{}, line, col++});
    } else if (c == '-') {
      tokens.push_back({TokenType::minus, std::monostate{}, line, col++});
    } else if (c == '*') {
      tokens.push_back({TokenType::star, std::monostate{}, line, col++});
    } else if (c == '/') {
      if (i + 1 < src.length() && src[i + 1] == '/') {
        i++; // Skip the second '/'
        col += 2;
        while (i + 1 < src.length() && src[i + 1] != '\n') {
          i++;
          col++;
        }
      } else {
        tokens.push_back({TokenType::slash, std::monostate{}, line, col++});
      }
    } else if (c == '{') {
      tokens.push_back({TokenType::open_curly, std::monostate{}, line, col++});
    } else if (c == '}') {
      tokens.push_back({TokenType::close_curly, std::monostate{}, line, col++});
    } else if (c == '(') {
      tokens.push_back({TokenType::open_paren, std::monostate{}, line, col++});
    } else if (c == ')') {
      tokens.push_back({TokenType::close_paren, std::monostate{}, line, col++});
    } else if (c == ',') {
      tokens.push_back({TokenType::comma, std::monostate{}, line, col++});
    } else if (c == '[') {
      tokens.push_back(
          {TokenType::open_bracket, std::monostate{}, line, col++});
    } else if (c == ']') {
      tokens.push_back(
          {TokenType::close_bracket, std::monostate{}, line, col++});
    } else if (std::isspace(c)) {
      if (c == '\n') {
        line++;
        col = 1;
      } else {
        col++;
      }
      continue;
    } else {
      std::cerr << "Error: Unknown character '" << c << "' at " << line << ":"
                << col << std::endl;
      exit(1);
    }
  }
  return tokens;
}
