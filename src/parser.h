#pragma once
#include "lexer.h"
#include <vector>
#include <memory>
#include <optional>
#include <string>

struct Node {
    virtual ~Node() = default;
    virtual void print(int indent = 0) const = 0;
};

struct Expr : public Node {};

struct IntLitExpr : public Expr {
    int value;
    IntLitExpr(int v) : value(v) {}
    void print(int indent = 0) const override;
};

struct IdentifierExpr : public Expr {
    std::string name;
    IdentifierExpr(std::string n) : name(std::move(n)) {}
    void print(int indent = 0) const override;
};

struct Stmt : public Node {};

struct ReturnStmt : public Stmt {
    std::unique_ptr<Expr> expr;
    ReturnStmt(std::unique_ptr<Expr> e);
    void print(int indent = 0) const override;
};

struct VarDecl : public Stmt {
    std::string name;
    std::unique_ptr<Expr> init;
    VarDecl(std::string n, std::unique_ptr<Expr> i) : name(std::move(n)), init(std::move(i)) {}
    void print(int indent = 0) const override;
};

struct Program : public Node {
    std::vector<std::unique_ptr<Stmt>> stmts;
    void print(int indent = 0) const override;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse_program();
    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<Expr> parse_expr();

private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
};
