#include "parser.h"
#include <iostream>
#include <iomanip>

// Helper for indentation
void print_indent(int indent) {
    std::cout << std::string(indent * 2, ' ');
}

void IntLitExpr::print(int indent) const {
    print_indent(indent);
    std::cout << "IntLit(" << value << ")" << std::endl;
}

void IdentifierExpr::print(int indent) const {
    print_indent(indent);
    std::cout << "Ident(" << name << ")" << std::endl;
}

void BinaryExpr::print(int indent) const {
    print_indent(indent);
    std::string op_char;
    switch(op) {
        case TokenType::plus: op_char = "+"; break;
        case TokenType::minus: op_char = "-"; break;
        case TokenType::star: op_char = "*"; break;
        case TokenType::slash: op_char = "/"; break;
        default: op_char = "?"; break;
    }
    std::cout << "BinaryExpr(" << op_char << "):" << std::endl;
    lhs->print(indent + 1);
    rhs->print(indent + 1);
}

ReturnStmt::ReturnStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}

void ReturnStmt::print(int indent) const {
    print_indent(indent);
    std::cout << "ReturnStmt:" << std::endl;
    expr->print(indent + 1);
}

void VarDecl::print(int indent) const {
    print_indent(indent);
    std::cout << "VarDecl(" << name << "):" << std::endl;
    init->print(indent + 1);
}

void AssignStmt::print(int indent) const {
    print_indent(indent);
    std::cout << "AssignStmt(" << name << "):" << std::endl;
    value->print(indent + 1);
}

void ScopeStmt::print(int indent) const {
    print_indent(indent);
    std::cout << "ScopeStmt:" << std::endl;
    for (const auto& stmt : stmts) {
        stmt->print(indent + 1);
    }
}

void IfStmt::print(int indent) const {
    print_indent(indent);
    std::cout << "IfStmt:" << std::endl;
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
    std::cout << "WhileStmt:" << std::endl;
    print_indent(indent + 1);
    std::cout << "Condition:" << std::endl;
    condition->print(indent + 2);
    print_indent(indent + 1);
    std::cout << "Body:" << std::endl;
    body->print(indent + 2);
}

void Program::print(int indent) const {
    print_indent(indent);
    std::cout << "Program:" << std::endl;
    for (const auto& stmt : stmts) {
        stmt->print(indent + 1);
    }
}

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<Expr> Parser::parse_expr() {
    return parse_additive();
}

std::unique_ptr<Expr> Parser::parse_additive() {
    auto lhs = parse_term();
    while (peek().has_value()) {
        if (peek().value().type == TokenType::plus || peek().value().type == TokenType::minus) {
            Token op = consume();
            auto rhs = parse_term();
            if (!rhs) {
                std::cerr << "Error: Expected expression after operator" << std::endl;
                exit(1);
            }
            lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type);
        } else {
            break;
        }
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::parse_term() {
    auto lhs = parse_factor();
    while (peek().has_value()) {
        if (peek().value().type == TokenType::star || peek().value().type == TokenType::slash) {
            Token op = consume();
            auto rhs = parse_factor();
            if (!rhs) {
                std::cerr << "Error: Expected expression after operator" << std::endl;
                exit(1);
            }
            lhs = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), op.type);
        } else {
            break;
        }
    }
    return lhs;
}

std::unique_ptr<Expr> Parser::parse_factor() {
    if (peek().has_value()) {
        if (peek().value().type == TokenType::int_lit) {
            auto token = consume();
            return std::make_unique<IntLitExpr>(std::get<int>(token.value));
        } else if (peek().value().type == TokenType::ident) {
            auto token = consume();
            return std::make_unique<IdentifierExpr>(std::get<std::string>(token.value));
        } else if (peek().value().type == TokenType::open_paren) {
            consume(); // Eat '('
            auto expr = parse_expr();
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume(); // Eat ')'
                return expr;
            } else {
                std::cerr << "Error: Expected ')'" << std::endl;
                exit(1);
            }
        }
    }
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_scope() {
    if (peek().has_value() && peek().value().type == TokenType::open_curly) {
        consume(); // Eat '{'
        std::vector<std::unique_ptr<Stmt>> stmts;
        while (peek().has_value() && peek().value().type != TokenType::close_curly) {
            auto stmt = parse_stmt();
            if (stmt) {
                stmts.push_back(std::move(stmt));
            } else {
                break;
            }
        }
        if (peek().has_value() && peek().value().type == TokenType::close_curly) {
            consume(); // Eat '}'
            return std::make_unique<ScopeStmt>(std::move(stmts));
        } else {
            std::cerr << "Error: Expected '}'" << std::endl;
            exit(1);
        }
    }
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    if (!peek().has_value()) return nullptr;

    if (peek().value().type == TokenType::_return) {
        consume();
        auto expr = parse_expr();
        if (!expr) {
            std::cerr << "Error: Expected expression after 'return'" << std::endl;
            exit(1);
        }
        if (peek().has_value() && peek().value().type == TokenType::semi) {
            consume();
        } else {
            std::cerr << "Error: Expected ';' after return statement" << std::endl;
            exit(1);
        }
        return std::make_unique<ReturnStmt>(std::move(expr));
    } 
    else if (peek().value().type == TokenType::_int) {
        consume();
        if (peek().has_value() && peek().value().type == TokenType::ident) {
            auto name_token = consume();
            std::string name = std::get<std::string>(name_token.value);
            if (peek().has_value() && peek().value().type == TokenType::eq) {
                consume();
                auto init = parse_expr();
                if (!init) {
                    std::cerr << "Error: Expected expression after '='" << std::endl;
                    exit(1);
                }
                if (peek().has_value() && peek().value().type == TokenType::semi) {
                    consume();
                } else {
                    std::cerr << "Error: Expected ';' after variable declaration" << std::endl;
                    exit(1);
                }
                return std::make_unique<VarDecl>(name, std::move(init));
            } else {
                std::cerr << "Error: Expected '=' in variable declaration" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Error: Expected identifier after 'int'" << std::endl;
            exit(1);
        }
    }
    else if (peek().value().type == TokenType::ident) {
        // Lookahead to see if it's assignment
        if (peek(1).has_value() && peek(1).value().type == TokenType::eq) {
            auto name_token = consume();
            std::string name = std::get<std::string>(name_token.value);
            consume(); // Eat '='
            auto expr = parse_expr();
            if (!expr) {
                std::cerr << "Error: Expected expression after '='" << std::endl;
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume();
            } else {
                std::cerr << "Error: Expected ';' after assignment" << std::endl;
                exit(1);
            }
            return std::make_unique<AssignStmt>(name, std::move(expr));
        } else {
            std::cerr << "Error: Unexpected identifier or missing assignment." << std::endl;
            exit(1);
        }
    }
    else if (peek().value().type == TokenType::open_curly) {
        return parse_scope();
    }
    else if (peek().value().type == TokenType::_if) {
        consume(); // Eat 'if'
        if (peek().has_value() && peek().value().type == TokenType::open_paren) {
            consume(); // Eat '('
            auto condition = parse_expr();
            if (!condition) {
                std::cerr << "Error: Expected expression in if condition" << std::endl;
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume(); // Eat ')'
                auto then_stmt = parse_stmt();
                if (!then_stmt) {
                    std::cerr << "Error: Expected statement after if condition" << std::endl;
                    exit(1);
                }
                
                std::unique_ptr<Stmt> else_stmt = nullptr;
                if (peek().has_value() && peek().value().type == TokenType::_else) {
                    consume(); // Eat 'else'
                    else_stmt = parse_stmt();
                }
                
                return std::make_unique<IfStmt>(std::move(condition), std::move(then_stmt), std::move(else_stmt));
            } else {
                std::cerr << "Error: Expected ')' after if condition" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Error: Expected '(' after if" << std::endl;
            exit(1);
        }
    }
    else if (peek().value().type == TokenType::_while) {
        consume(); // Eat 'while'
        if (peek().has_value() && peek().value().type == TokenType::open_paren) {
            consume(); // Eat '('
            auto condition = parse_expr();
            if (!condition) {
                std::cerr << "Error: Expected expression in while condition" << std::endl;
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::close_paren) {
                consume(); // Eat ')'
                auto body = parse_stmt();
                if (!body) {
                    std::cerr << "Error: Expected statement after while condition" << std::endl;
                    exit(1);
                }
                return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
            } else {
                std::cerr << "Error: Expected ')' after while condition" << std::endl;
                exit(1);
            }
        } else {
            std::cerr << "Error: Expected '(' after while" << std::endl;
            exit(1);
        }
    }

    return nullptr;
}

std::unique_ptr<Program> Parser::parse_program() {
    auto program = std::make_unique<Program>();
    while (peek().has_value()) {
        if (auto stmt = parse_stmt()) {
            program->stmts.push_back(std::move(stmt));
        } else {
            std::cerr << "Error: Unexpected token" << std::endl;
            exit(1);
        }
    }
    return program;
}

std::optional<Token> Parser::peek(int offset) const {
    if (m_index + offset >= m_tokens.size()) return std::nullopt;
    return m_tokens[m_index + offset];
}

Token Parser::consume() {
    return m_tokens[m_index++];
}