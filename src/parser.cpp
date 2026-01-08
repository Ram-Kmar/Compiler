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

void Program::print(int indent) const {
    print_indent(indent);
    std::cout << "Program:" << std::endl;
    for (const auto& stmt : stmts) {
        stmt->print(indent + 1);
    }
}

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::unique_ptr<Expr> Parser::parse_expr() {
    if (peek().has_value()) {
        if (peek().value().type == TokenType::int_lit) {
            auto token = consume();
            return std::make_unique<IntLitExpr>(std::get<int>(token.value));
        } else if (peek().value().type == TokenType::ident) {
            auto token = consume();
            return std::make_unique<IdentifierExpr>(std::get<std::string>(token.value));
        }
    }
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    if (peek().has_value()) {
        if (peek().value().type == TokenType::_return) {
            consume(); // Eat 'return'
            auto expr = parse_expr();
            if (!expr) {
                std::cerr << "Error: Expected expression after 'return'" << std::endl;
                exit(1);
            }
            if (peek().has_value() && peek().value().type == TokenType::semi) {
                consume(); // Eat ';'
            } else {
                std::cerr << "Error: Expected ';' after return statement" << std::endl;
                exit(1);
            }
            return std::make_unique<ReturnStmt>(std::move(expr));
        } else if (peek().value().type == TokenType::_int) {
            consume(); // Eat 'int'
            if (peek().has_value() && peek().value().type == TokenType::ident) {
                auto name_token = consume();
                std::string name = std::get<std::string>(name_token.value);
                if (peek().has_value() && peek().value().type == TokenType::eq) {
                    consume(); // Eat '='
                    auto init = parse_expr();
                    if (!init) {
                        std::cerr << "Error: Expected expression after '='" << std::endl;
                        exit(1);
                    }
                    if (peek().has_value() && peek().value().type == TokenType::semi) {
                        consume(); // Eat ';'
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