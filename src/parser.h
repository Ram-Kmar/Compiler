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

struct CallExpr : public Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr(std::string c, std::vector<std::unique_ptr<Expr>> a) 
        : callee(std::move(c)), args(std::move(a)) {}
    void print(int indent = 0) const override;
};

struct BinaryExpr : public Expr {
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
    TokenType op; 
    BinaryExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, TokenType o) 
        : lhs(std::move(l)), rhs(std::move(r)), op(o) {}
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

struct AssignStmt : public Stmt {
    std::string name;
    std::unique_ptr<Expr> value;
    AssignStmt(std::string n, std::unique_ptr<Expr> v) : name(std::move(n)), value(std::move(v)) {}
    void print(int indent = 0) const override;
};

struct ScopeStmt : public Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    ScopeStmt(std::vector<std::unique_ptr<Stmt>> s) : stmts(std::move(s)) {}
    void print(int indent = 0) const override;
};

struct IfStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_stmt;
    std::unique_ptr<Stmt> else_stmt; 
    IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t, std::unique_ptr<Stmt> e = nullptr) 
        : condition(std::move(c)), then_stmt(std::move(t)), else_stmt(std::move(e)) {}
    void print(int indent = 0) const override;
};

struct WhileStmt : public Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b) 
        : condition(std::move(c)), body(std::move(b)) {}
    void print(int indent = 0) const override;
};

struct Arg {
    std::string name;
};

struct Function : public Node {
    std::string name;
    std::vector<Arg> args;
    std::unique_ptr<ScopeStmt> body;
    
    Function(std::string n, std::vector<Arg> a, std::unique_ptr<ScopeStmt> b) 
        : name(std::move(n)), args(std::move(a)), body(std::move(b)) {}
    void print(int indent = 0) const override;
};

struct Program : public Node {
    std::vector<std::unique_ptr<Function>> functions;
    void print(int indent = 0) const override;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse_program();
    
private:
    const std::vector<Token> m_tokens;
    size_t m_index = 0;
    std::optional<Token> peek(int offset = 0) const;
    Token consume();
    
    std::unique_ptr<Function> parse_function();
    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<Stmt> parse_scope(); 
    
    std::unique_ptr<Expr> parse_expr();
    std::unique_ptr<Expr> parse_comparison();
    std::unique_ptr<Expr> parse_additive();
    std::unique_ptr<Expr> parse_term();
    std::unique_ptr<Expr> parse_factor();
};