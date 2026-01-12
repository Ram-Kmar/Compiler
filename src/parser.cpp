#include "parser.h"
#include <iomanip>
#include <iostream>

void print_indent(int indent) { std::cout << std::string(indent * 2, ' '); }

void IntLitExpr::print(int indent) const {
  print_indent(indent);
  std::cout << "IntLit(" << value << ") at " << line << ":" << col << std::endl;
}

void BoolLitExpr::print(int indent) const {
  print_indent(indent);
  std::cout << "BoolLit(" << (value ? "true" : "false") << ") at " << line << ":" << col << std::endl;
}

void IdentifierExpr::print(int indent) const {
  print_indent(indent);
  std::cout << "Ident(" << name << ") at " << line << ":" << col << std::endl;
}

void ArrayAccessExpr::print(int indent) const {
  print_indent(indent);
  std::cout << "ArrayAccess(" << name << ") at " << line << ":" << col << ":" << std::endl;
  index->print(indent + 1);
}

void CallExpr::print(int indent) const {
  print_indent(indent);
  std::cout << "CallExpr(" << callee << ") at " << line << ":" << col << ":" << std::endl;
  for (const auto &arg : args) {
    arg->print(indent + 1);
  }
}

void UnaryExpr::print(int indent) const {
  print_indent(indent);
  std::string op_char;
  switch (op) {
  case TokenType::bang:
    op_char = "!";
    break;
  default:
    op_char = "?";
    break;
  }
  std::cout << "UnaryExpr(" << op_char << ") at " << line << ":" << col << ":" << std::endl;
  operand->print(indent + 1);
}

void BinaryExpr::print(int indent) const {
  print_indent(indent);
  std::string op_char;
  switch (op) {
  case TokenType::plus:
    op_char = "+";
    break;
  case TokenType::minus:
    op_char = "-";
    break;
  case TokenType::star:
    op_char = "*";
    break;
  case TokenType::slash:
    op_char = "/";
    break;
  case TokenType::eq_eq:
    op_char = "==";
    break;
  case TokenType::neq:
    op_char = "!=";
    break;
  case TokenType::lt:
    op_char = "<";
    break;
  case TokenType::gt:
    op_char = ">";
    break;
  case TokenType::amp_amp:
    op_char = "&&";
    break;
  case TokenType::pipe_pipe:
    op_char = "||";
    break;
  default:
    op_char = "?";
    break;
  }
  std::cout << "BinaryExpr(" << op_char << ") at " << line << ":" << col << ":" << std::endl;
  lhs->print(indent + 1);
  rhs->print(indent + 1);
}

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> e, int l, int c) : Stmt(l, c), expr(std::move(e)) {}

void ReturnStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "ReturnStmt at " << line << ":" << col << ":" << std::endl;
  expr->print(indent + 1);
}

void ExprStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "ExprStmt at " << line << ":" << col << ":" << std::endl;
  expr->print(indent + 1);
}

void VarDecl::print(int indent) const {
  print_indent(indent);
  std::string type_str = "void";
  if (type.base == Type::Base::Int) type_str = "int";
  else if (type.base == Type::Base::Bool) type_str = "bool";
  for (int i=0; i<type.ptr_level; ++i) type_str += "*";

  if (array_size) {
    std::cout << "VarDecl(" << name << "[" << *array_size << "], " << type_str << ") at " << line << ":" << col << ":" << std::endl;
  } else {
    std::cout << "VarDecl(" << name << ", " << type_str << ") at " << line << ":" << col << ":" << std::endl;
  }
  if (init) init->print(indent + 1);
}

void AssignStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "AssignStmt(" << name << ") at " << line << ":" << col << ":" << std::endl;
  value->print(indent + 1);
}

void ArrayAssignStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "ArrayAssignStmt(" << name << ") at " << line << ":" << col << ":" << std::endl;
  print_indent(indent + 1);
  std::cout << "Index:" << std::endl;
  index->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Value:" << std::endl;
  value->print(indent + 2);
}

void PointerAssignStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "PointerAssignStmt at " << line << ":" << col << ":" << std::endl;
  print_indent(indent + 1);
  std::cout << "Ptr:" << std::endl;
  ptr_expr->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Value:" << std::endl;
  value->print(indent + 2);
}

void ScopeStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "ScopeStmt at " << line << ":" << col << ":" << std::endl;
  for (const auto &stmt : stmts) {
    stmt->print(indent + 1);
  }
}

void IfStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "IfStmt at " << line << ":" << col << ":" << std::endl;
  print_indent(indent + 1);
  std::cout << "Condition:" << std::endl;
  condition->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Then:" << std::endl;
  then_stmt->print(indent + 2);
  if (else_stmt) {
    print_indent(indent + 1);
    std::cout << "Else:" << std::endl;
    else_stmt->print(indent + 2);
  }
}

void WhileStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "WhileStmt at " << line << ":" << col << ":" << std::endl;
  print_indent(indent + 1);
  std::cout << "Condition:" << std::endl;
  condition->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Body:" << std::endl;
  body->print(indent + 2);
}

void ForStmt::print(int indent) const {
  print_indent(indent);
  std::cout << "ForStmt at " << line << ":" << col << ":" << std::endl;
  print_indent(indent + 1);
  std::cout << "Init:" << std::endl;
  if (init) init->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Condition:" << std::endl;
  if (condition) condition->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Increment:" << std::endl;
  if (increment) increment->print(indent + 2);
  print_indent(indent + 1);
  std::cout << "Body:" << std::endl;
  body->print(indent + 2);
}

void Function::print(int indent) const {
  print_indent(indent);
  std::string ret_type_str = "void";
  if (return_type.base == Type::Base::Int) ret_type_str = "int";
  else if (return_type.base == Type::Base::Bool) ret_type_str = "bool";
  for (int i=0; i<return_type.ptr_level; ++i) ret_type_str += "*";

  std::cout << "Function(" << name << ", " << ret_type_str << ") at " << line << ":" << col << ":" << std::endl;
  for (const auto &arg : args) {
    print_indent(indent + 1);
    std::string arg_type_str = "void";
    if (arg.type.base == Type::Base::Int) arg_type_str = "int";
    else if (arg.type.base == Type::Base::Bool) arg_type_str = "bool";
    for (int i=0; i<arg.type.ptr_level; ++i) arg_type_str += "*";
    std::cout << "Arg(" << arg.name << ", " << arg_type_str << ")" << std::endl;
  }
  body->print(indent + 1);
}

void Program::print(int indent) const {
  print_indent(indent);
  std::cout << "Program:" << std::endl;
  for (const auto &stmt : globals) {
    stmt->print(indent + 1);
  }
  for (const auto &func : functions) {
    func->print(indent + 1);
  }
}

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

void Parser::report_error(const std::string& message, std::optional<Token> token) const {
    if (token) {
        std::cerr << "Parser Error: " << message << " at " << token->line << ":" << token->col << std::endl;
    } else if (peek().has_value()) {
        std::cerr << "Parser Error: " << message << " at " << peek()->line << ":" << peek()->col << std::endl;
    } else if (!m_tokens.empty()) {
        const auto& last = m_tokens.back();
        std::cerr << "Parser Error: " << message << " at end of file (after " << last.line << ":" << last.col << ")" << std::endl;
    } else {
        std::cerr << "Parser Error: " << message << " at start of file" << std::endl;
    }
    exit(1);
}

std::unique_ptr<Expr>
Parser::parse_expr() {
  return parse_logical_or();
}

std::unique_ptr<Expr> Parser::parse_logical_or() {
  auto lhs = parse_logical_and();
  while (peek().has_value()) {
    if (peek().value().type == TokenType::pipe_pipe) {
      Token op = consume();
      auto rhs = parse_logical_and();
      if (!rhs) {
        report_error("Expected expression after '||'", op);
      }
      lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type, op.line, op.col);
    } else {
      break;
    }
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parse_logical_and() {
  auto lhs = parse_comparison();
  while (peek().has_value()) {
    if (peek().value().type == TokenType::amp_amp) {
      Token op = consume();
      auto rhs = parse_comparison();
      if (!rhs) {
        report_error("Expected expression after '&&'", op);
      }
      lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type, op.line, op.col);
    } else {
      break;
    }
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parse_comparison() {
  auto lhs = parse_additive();
  while (peek().has_value()) {
    if (peek().value().type == TokenType::eq_eq ||
        peek().value().type == TokenType::neq ||
        peek().value().type == TokenType::lt ||
        peek().value().type == TokenType::gt) {
      Token op = consume();
      auto rhs = parse_additive();
      if (!rhs) {
        report_error("Expected expression after operator", op);
      }
      lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type, op.line, op.col);
    } else {
      break;
    }
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parse_additive() {
  auto lhs = parse_term();
  while (peek().has_value()) {
    if (peek().value().type == TokenType::plus ||
        peek().value().type == TokenType::minus) {
      Token op = consume();
      auto rhs = parse_term();
      if (!rhs) {
        report_error("Expected expression after operator", op);
      }
      lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type, op.line, op.col);
    } else {
      break;
    }
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parse_term() {
  auto lhs = parse_unary();
  while (peek().has_value()) {
    if (peek().value().type == TokenType::star ||
        peek().value().type == TokenType::slash) {
      Token op = consume();
      auto rhs = parse_unary();
      if (!rhs) {
        report_error("Expected expression after operator", op);
      }
      lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type, op.line, op.col);
    } else {
      break;
    }
  }
  return lhs;
}

std::unique_ptr<Expr> Parser::parse_unary() {
  if (peek().has_value()) {
      if (peek().value().type == TokenType::bang) {
        Token op = consume();
        auto operand = parse_unary();
        if (!operand) {
            report_error("Expected expression after '!'", op);
        }
        return std::make_unique<UnaryExpr>(std::move(operand), op.type, op.line, op.col);
      } else if (peek().value().type == TokenType::star) {
        Token op = consume();
        auto operand = parse_unary(); // Right-associative: **x -> *(*x)
        if (!operand) {
            report_error("Expected expression after '*'", op);
        }
        return std::make_unique<UnaryExpr>(std::move(operand), op.type, op.line, op.col);
      } else if (peek().value().type == TokenType::amp) {
        Token op = consume();
        auto operand = parse_unary();
        if (!operand) {
            report_error("Expected expression after '&'", op);
        }
        return std::make_unique<UnaryExpr>(std::move(operand), op.type, op.line, op.col);
      }
  }
  return parse_factor();
}

std::unique_ptr<Expr> Parser::parse_factor() {
  if (peek().has_value()) {
    if (peek().value().type == TokenType::int_lit) {
      auto token = consume();
      return std::make_unique<IntLitExpr>(std::get<int>(token.value), token.line, token.col);
    } else if (peek().value().type == TokenType::_true) {
      auto token = consume();
      return std::make_unique<BoolLitExpr>(true, token.line, token.col);
    } else if (peek().value().type == TokenType::_false) {
      auto token = consume();
      return std::make_unique<BoolLitExpr>(false, token.line, token.col);
    } else if (peek().value().type == TokenType::ident) {
      // Check for CallExpr
      if (peek(1).has_value() &&
          peek(1).value().type == TokenType::open_paren) {
        auto token = consume();
        std::string name = std::get<std::string>(token.value);
        consume(); // Eat '('
        std::vector<std::unique_ptr<Expr>> args;
        if (peek().has_value() &&
            peek().value().type != TokenType::close_paren) {
          while (true) {
            args.push_back(parse_expr());
            if (peek().has_value() && peek().value().type == TokenType::comma) {
              consume();
            } else {
              break;
            }
          }
        }
        if (peek().has_value() &&
            peek().value().type == TokenType::close_paren) {
          consume(); // Eat ')'
          return std::make_unique<CallExpr>(name, std::move(args), token.line, token.col);
        } else {
          report_error("Expected ')' after function call arguments");
        }
      } else if (peek(1).has_value() &&
                 peek(1).value().type == TokenType::open_bracket) {
        auto token = consume();
        std::string name = std::get<std::string>(token.value);
        consume(); // Eat '['
        auto index = parse_expr();
        if (peek().has_value() && peek().value().type == TokenType::close_bracket) {
            consume(); // Eat ']'
            return std::make_unique<ArrayAccessExpr>(name, std::move(index), token.line, token.col);
        } else {
            report_error("Expected ']' after array index");
        }
      } else {
        auto token = consume();
        return std::make_unique<IdentifierExpr>(
            std::get<std::string>(token.value), token.line, token.col);
      }
    } else if (peek().value().type == TokenType::open_paren) {
      consume(); // Eat '('
      auto expr = parse_expr();
      if (peek().has_value() && peek().value().type == TokenType::close_paren) {
        consume(); // Eat ')'
        return expr;
      } else {
        report_error("Expected ')'");
      }
    }
  }
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_scope() {
  if (peek().has_value() && peek().value().type == TokenType::open_curly) {
    auto start_token = consume(); // Eat '{'
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (peek().has_value() &&
           peek().value().type != TokenType::close_curly) {
      auto stmt = parse_stmt();
      if (stmt) {
        stmts.push_back(std::move(stmt));
      } else {
        break;
      }
    }
    if (peek().has_value() && peek().value().type == TokenType::close_curly) {
      consume(); // Eat '}'
      return std::make_unique<ScopeStmt>(std::move(stmts), start_token.line, start_token.col);
    } else {
      report_error("Expected '}'");
    }
  }
  return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
  if (!peek().has_value())
    return nullptr;

  if (peek().value().type == TokenType::_return) {
    auto start_token = consume();
    auto expr = parse_expr();
    if (!expr) {
      report_error("Expected expression after 'return'", start_token);
    }
    if (peek().has_value() && peek().value().type == TokenType::semi) {
      consume();
    } else {
      report_error("Expected ';' after return statement");
    }
    return std::make_unique<ReturnStmt>(std::move(expr), start_token.line, start_token.col);
  } else if (peek().value().type == TokenType::_int || peek().value().type == TokenType::_bool) {
    auto type_token = consume();
    Type type = (type_token.type == TokenType::_int) ? Type::Int() : Type::Bool();
    
    // Parse pointer levels
    while (peek().has_value() && peek().value().type == TokenType::star) {
        consume();
        type.ptr_level++;
    }
    
    std::optional<int> array_size;
    if (peek().has_value() && peek().value().type == TokenType::open_bracket) {
        consume(); // Eat '['
        if (peek().has_value() && peek().value().type == TokenType::int_lit) {
            array_size = std::get<int>(consume().value);
            if (peek().has_value() && peek().value().type == TokenType::close_bracket) {
                consume(); // Eat ']'
            } else { report_error("Expected ']' after array size"); }
        } else { report_error("Expected integer literal for array size"); }
    }

    if (peek().has_value() && peek().value().type == TokenType::ident) {
      auto name_token = consume();
      std::string name = std::get<std::string>(name_token.value);
      if (peek().has_value() && peek().value().type == TokenType::eq) {
        consume();
        auto init = parse_expr();
        if (!init) {
          report_error("Expected expression after '='");
        }
        if (peek().has_value() && peek().value().type == TokenType::semi) {
          consume();
        } else {
          report_error("Expected ';' after variable declaration");
        }
        return std::make_unique<VarDecl>(name, type, std::move(init), type_token.line, type_token.col, array_size);
      } else if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
        return std::make_unique<VarDecl>(name, type, nullptr, type_token.line, type_token.col, array_size);
      } else {
        report_error("Expected '=' or ';' in variable declaration");
      }
    } else {
      report_error("Expected identifier after type");
    }
  } else if (peek().value().type == TokenType::ident) {
    auto start_token = peek().value();
    // Lookahead to see if it's assignment or call
    if (peek(1).has_value() && peek(1).value().type == TokenType::eq) {
      auto name_token = consume();
      std::string name = std::get<std::string>(name_token.value);
      consume(); // Eat '='
      auto expr = parse_expr();
      if (!expr) {
        report_error("Expected expression after '='");
      }
      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
      } else {
        report_error("Expected ';' after assignment");
      }
      return std::make_unique<AssignStmt>(name, std::move(expr), start_token.line, start_token.col);
    } else if (peek(1).has_value() && peek(1).value().type == TokenType::open_bracket) {
      auto name_token = consume();
      std::string name = std::get<std::string>(name_token.value);
      consume(); // Eat '['
      auto index = parse_expr();
      if (peek().has_value() && peek().value().type == TokenType::close_bracket) {
          consume(); // Eat ']'
          if (peek().has_value() && peek().value().type == TokenType::eq) {
              consume(); // Eat '='
              auto value = parse_expr();
              if (peek().has_value() && peek().value().type == TokenType::semi) {
                  consume();
                  return std::make_unique<ArrayAssignStmt>(name, std::move(index), std::move(value), start_token.line, start_token.col);
              } else { report_error("Expected ';' after array assignment"); }
          } else { report_error("Expected '=' after array index"); }
      } else { report_error("Expected ']' after array index"); }
    } else if (peek(1).has_value() &&
               peek(1).value().type == TokenType::open_paren) {
      auto expr = parse_expr();
      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume();
      } else {
        report_error("Expected ';' after expression statement");
      }
      return std::make_unique<ExprStmt>(std::move(expr), start_token.line, start_token.col);
    } else {
      report_error("Unexpected identifier or missing assignment.");
    }
  } else if (peek().value().type == TokenType::star) { // *p = 10;
    auto start_token = consume(); // Eat '*'
    // This is tricky. parse_expr() would parse *p. 
    // We need to verify we have an expression that ends with assignment.
    // Actually, we can just say: if we see *, parse a UnaryExpr. 
    // BUT parse_stmt needs to distinguish *p (expr stmt?) vs *p = 10 (assign stmt).
    // Let's manually parse the pointer part.
    // It must be a UnaryExpr (dereference).
    // Or we can cheat: parse_expr() for the LHS, check if it is valid for assignment, then look for '='.
    // But parse_expr eats logical ORs etc.
    // Let's assume *<expr> = <expr>;
    // We can use parse_unary() to get the LHS *ptr.
    // But we consumed the star already.
    auto ptr_expr = parse_unary(); // This parses 'p' in '*p'.
    // So LHS is UnaryExpr(ptr_expr, star).
    // Now check for =
    if (peek().has_value() && peek().value().type == TokenType::eq) {
        consume(); // Eat '='
        auto value = parse_expr();
        if (peek().has_value() && peek().value().type == TokenType::semi) {
            consume();
        } else { report_error("Expected ';' after pointer assignment"); }
        return std::make_unique<PointerAssignStmt>(std::move(ptr_expr), std::move(value), start_token.line, start_token.col);
    } else {
         report_error("Expected '=' after pointer dereference in statement");
    }
  } else if (peek().value().type == TokenType::open_curly) {
    return parse_scope();
  } else if (peek().value().type == TokenType::_if) {
    auto start_token = consume(); // Eat 'if'
    if (peek().has_value() && peek().value().type == TokenType::open_paren) {
      consume(); // Eat '('
      auto condition = parse_expr();
      if (!condition) {
        report_error("Expected expression in if condition");
      }
      if (peek().has_value() && peek().value().type == TokenType::close_paren) {
        consume(); // Eat ')'
        auto then_stmt = parse_stmt();
        if (!then_stmt) {
          report_error("Expected statement after if condition");
        }

        std::unique_ptr<Stmt> else_stmt = nullptr;
        if (peek().has_value() && peek().value().type == TokenType::_else) {
          consume(); // Eat 'else'
          else_stmt = parse_stmt();
        }

        return std::make_unique<IfStmt>(
            std::move(condition), std::move(then_stmt), std::move(else_stmt), start_token.line, start_token.col);
      } else {
        report_error("Expected ')' after if condition");
      }
    } else {
      report_error("Expected '(' after if");
    }
  } else if (peek().value().type == TokenType::_while) {
    auto start_token = consume(); // Eat 'while'
    if (peek().has_value() && peek().value().type == TokenType::open_paren) {
      consume(); // Eat '('
      auto condition = parse_expr();
      if (!condition) {
        report_error("Expected expression in while condition");
      }
      if (peek().has_value() && peek().value().type == TokenType::close_paren) {
        consume(); // Eat ')'
        auto body = parse_stmt();
        if (!body) {
          report_error("Expected statement after while condition");
        }
        return std::make_unique<WhileStmt>(std::move(condition),
                                           std::move(body), start_token.line, start_token.col);
      } else {
        report_error("Expected ')' after while condition");
      }
    } else {
      report_error("Expected '(' after while");
    }
  } else if (peek().value().type == TokenType::_for) {
    auto start_token = consume(); // Eat 'for'
    if (peek().has_value() && peek().value().type == TokenType::open_paren) {
      consume(); // Eat '('
      
      // 1. Init
      std::unique_ptr<Stmt> init = nullptr;
      if (peek().has_value() && peek().value().type != TokenType::semi) {
        if (peek().value().type == TokenType::_int || peek().value().type == TokenType::_bool) {
            auto type_token = consume();
            Type type = (type_token.type == TokenType::_int) ? Type::Int() : Type::Bool();
            if (peek().has_value() && peek().value().type == TokenType::ident) {
                auto name_token = consume();
                std::string name = std::get<std::string>(name_token.value);
                if (peek().has_value() && peek().value().type == TokenType::eq) {
                    consume();
                    auto init_expr = parse_expr();
                    init = std::make_unique<VarDecl>(name, type, std::move(init_expr), type_token.line, type_token.col);
                } else { report_error("Expected '=' in for-init"); }
            } else { report_error("Expected identifier in for-init"); }
        } else if (peek().value().type == TokenType::ident) {
            auto name_token = consume();
            std::string name = std::get<std::string>(name_token.value);
            if (peek().has_value() && peek().value().type == TokenType::eq) {
                consume();
                auto val_expr = parse_expr();
                init = std::make_unique<AssignStmt>(name, std::move(val_expr), name_token.line, name_token.col);
            } else { report_error("Expected '=' in for-init"); }
        }
      }
      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume(); // Eat ';'
      } else { report_error("Expected ';' after for-init"); }

      // 2. Condition
      std::unique_ptr<Expr> condition = nullptr;
      if (peek().has_value() && peek().value().type != TokenType::semi) {
        condition = parse_expr();
      }
      if (peek().has_value() && peek().value().type == TokenType::semi) {
        consume(); // Eat ';'
      } else { report_error("Expected ';' after for-condition"); }

      // 3. Increment
      std::unique_ptr<Stmt> increment = nullptr;
      if (peek().has_value() && peek().value().type != TokenType::close_paren) {
        if (peek().value().type == TokenType::ident) {
            auto name_token = consume();
            std::string name = std::get<std::string>(name_token.value);
            if (peek().has_value() && peek().value().type == TokenType::eq) {
                consume();
                auto val_expr = parse_expr();
                increment = std::make_unique<AssignStmt>(name, std::move(val_expr), name_token.line, name_token.col);
            } else { report_error("Expected '=' in for-increment"); }
        } else {
            auto expr = parse_expr();
            increment = std::make_unique<ExprStmt>(std::move(expr), start_token.line, start_token.col);
        }
      }
      if (peek().has_value() && peek().value().type == TokenType::close_paren) {
        consume(); // Eat ')'
      } else { report_error("Expected ')' after for-increment"); }

      // 4. Body
      auto body = parse_stmt();
      if (!body) {
        report_error("Expected statement after for loop");
      }
      return std::make_unique<ForStmt>(std::move(init), std::move(condition), std::move(increment), std::move(body), start_token.line, start_token.col);
    } else {
      report_error("Expected '(' after for");
    }
  }

  return nullptr;
}

std::unique_ptr<Function> Parser::parse_function() {
  // Expect type
  Type return_type;
  auto start_token = peek().value();
  if (peek().has_value() && peek().value().type == TokenType::_int) {
    consume();
    return_type = Type::Int();
  } else if (peek().has_value() && peek().value().type == TokenType::_bool) {
    consume();
    return_type = Type::Bool();
  } else {
    report_error("Expected return type");
  }

  // Parse pointer levels for return type
  while (peek().has_value() && peek().value().type == TokenType::star) {
      consume();
      return_type.ptr_level++;
  }

  // Expect Identifier (Function Name)
  if (!peek().has_value() || peek().value().type != TokenType::ident) {
    report_error("Expected function name");
  }
  std::string name = std::get<std::string>(consume().value);

  // Expect '('
  if (!peek().has_value() || peek().value().type != TokenType::open_paren) {
    report_error("Expected '('");
  }
  consume();

  std::vector<Arg> args;
  // Parse args
  if (peek().has_value() && peek().value().type != TokenType::close_paren) {
    while (true) {
      Type arg_type;
      if (peek().has_value() && peek().value().type == TokenType::_int) {
        consume();
        arg_type = Type::Int();
      } else if (peek().has_value() && peek().value().type == TokenType::_bool) {
        consume();
        arg_type = Type::Bool();
      } else {
        report_error("Expected arg type");
      }
      
      // Parse pointer levels for arg type
      while (peek().has_value() && peek().value().type == TokenType::star) {
          consume();
          arg_type.ptr_level++;
      }

      if (!peek().has_value() || peek().value().type != TokenType::ident) {
        report_error("Expected arg name");
      }
      args.push_back({std::get<std::string>(consume().value), arg_type});

      if (peek().has_value() && peek().value().type == TokenType::comma) {
        consume();
      } else {
        break;
      }
    }
  }

  if (!peek().has_value() || peek().value().type != TokenType::close_paren) {
    report_error("Expected ')'");
  }
  consume();

  // Expect '{' -> parse_scope
  if (!peek().has_value() || peek().value().type != TokenType::open_curly) {
    report_error("Expected function body start '{'");
  }

  auto body_stmt = parse_scope();
  if (!body_stmt) {
    report_error("Failed to parse function body");
  }

  ScopeStmt *raw_scope = dynamic_cast<ScopeStmt *>(body_stmt.get());
  if (!raw_scope) {
    report_error("Function body is not a scope statement");
  }
  body_stmt.release(); // release ownership from old ptr
  std::unique_ptr<ScopeStmt> scope_ptr(raw_scope);

  return std::make_unique<Function>(name, args, std::move(scope_ptr), return_type, start_token.line, start_token.col);
}

std::unique_ptr<Program> Parser::parse_program() {
  auto program = std::make_unique<Program>();
  while (peek().has_value()) {
    bool is_func = false;
    bool is_type = peek(0).has_value() && (peek(0).value().type == TokenType::_int || peek(0).value().type == TokenType::_bool);
    
    if (is_type &&
        peek(1).has_value() && peek(1).value().type == TokenType::ident &&
        peek(2).has_value() && peek(2).value().type == TokenType::open_paren) {
      is_func = true;
    }

    if (is_func) {
      program->functions.push_back(parse_function());
    } else {
      program->globals.push_back(parse_stmt());
    }
  }
  return program;
}

std::optional<Token> Parser::peek(int offset) const {
  if (m_index + offset >= m_tokens.size())
    return std::nullopt;
  return m_tokens[m_index + offset];
}

Token Parser::consume() { return m_tokens[m_index++]; }