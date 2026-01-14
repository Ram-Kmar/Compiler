// parser.h
#pragma once
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>
#include "lexer.h"

struct Type {
    enum class Base { Void, Int, Bool, Char, Float, Double, Struct } base;
    std::string struct_name;
    int ptr_level = 0;

    Type() : base(Base::Void) {}
    static Type Int() { Type t; t.base = Base::Int; return t; }
    static Type Char() { Type t; t.base = Base::Char; return t; }
    static Type Float() { Type t; t.base = Base::Float; return t; }
    static Type Double() { Type t; t.base = Base::Double; return t; }
    static Type Struct(const std::string& name) { Type t; t.base = Base::Struct; t.struct_name = name; return t; }
};

struct Expr {
    int line, col;
    Expr(int l, int c) : line(l), col(c) {}
    virtual ~Expr() = default;
    virtual void print(int indent = 0) const = 0;
};

struct IntLitExpr : Expr {
    int value;
    IntLitExpr(int v, int l, int c) : Expr(l, c), value(v) {}
    void print(int indent = 0) const override;
};

struct StringLitExpr : Expr {
    std::string value;
    StringLitExpr(const std::string& v, int l, int c) : Expr(l, c), value(v) {}
    void print(int indent = 0) const override;
};

struct IdentifierExpr : Expr {
    std::string name;
    IdentifierExpr(const std::string& n, int l, int c) : Expr(l, c), name(n) {}
    void print(int indent = 0) const override;
};

struct BinaryExpr : Expr {
    std::unique_ptr<Expr> lhs, rhs;
    TokenType op;
    BinaryExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, TokenType o, int ln, int cl)
        : Expr(ln, cl), lhs(std::move(l)), rhs(std::move(r)), op(o) {}
    void print(int indent = 0) const override;
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr(const std::string& f, std::vector<std::unique_ptr<Expr>> a, int l, int c)
        : Expr(l, c), callee(f), args(std::move(a)) {}
    void print(int indent = 0) const override;
};

struct Stmt {
    int line, col;
    Stmt(int l, int c) : line(l), col(c) {}
    virtual ~Stmt() = default;
    virtual void print(int indent = 0) const = 0;
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> expr;
    ReturnStmt(std::unique_ptr<Expr> e, int l, int c) : Stmt(l, c), expr(std::move(e)) {}
    void print(int indent = 0) const override;
};

struct VarDecl : Stmt {
    std::string name;
    Type type;
    std::unique_ptr<Expr> init;
    VarDecl(const std::string& n, Type t, std::unique_ptr<Expr> i, int l, int c)
        : Stmt(l, c), name(n), type(t), init(std::move(i)) {}
    void print(int indent = 0) const override;
};

struct ScopeStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;
    ScopeStmt(std::vector<std::unique_ptr<Stmt>> s, int l, int c) : Stmt(l, c), stmts(std::move(s)) {}
    void print(int indent = 0) const override;
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_stmt, else_stmt;
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> ts, std::unique_ptr<Stmt> es, int l, int c)
        : Stmt(l, c), condition(std::move(cond)), then_stmt(std::move(ts)), else_stmt(std::move(es)) {}
    void print(int indent = 0) const override;
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b, int l, int c)
        : Stmt(l, c), condition(std::move(cond)), body(std::move(b)) {}
    void print(int indent = 0) const override;
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> increment, body;
    ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> inc, std::unique_ptr<Stmt> b, int l, int c)
        : Stmt(l, c), init(std::move(i)), condition(std::move(cond)), increment(std::move(inc)), body(std::move(b)) {}
    void print(int indent = 0) const override;
};

struct DoWhileStmt : Stmt {
    std::unique_ptr<Stmt> body;
    std::unique_ptr<Expr> condition;
    DoWhileStmt(std::unique_ptr<Stmt> b, std::unique_ptr<Expr> c, int l, int cl)
        : Stmt(l, cl), body(std::move(b)), condition(std::move(c)) {}
    void print(int indent = 0) const override;
};

struct CaseStmt {
    std::unique_ptr<Expr> value;
    std::unique_ptr<Stmt> body;
};

struct SwitchStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<CaseStmt> cases;
    SwitchStmt(std::unique_ptr<Expr> cond, std::vector<CaseStmt> cs, int l, int c)
        : Stmt(l, c), condition(std::move(cond)), cases(std::move(cs)) {}
    void print(int indent = 0) const override;
};

struct StructDecl : Stmt {
    std::string name;
    std::vector<std::pair<Type, std::string>> members;
    StructDecl(const std::string& n, std::vector<std::pair<Type, std::string>> ms, int l, int c)
        : Stmt(l, c), name(n), members(std::move(ms)) {}
    void print(int indent = 0) const override;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    ExprStmt(std::unique_ptr<Expr> e, int l, int c) : Stmt(l, c), expr(std::move(e)) {}
    void print(int indent = 0) const override;
};

struct Arg { std::string name; Type type; };

struct Function {
    std::string name;
    Type return_type;
    std::vector<Arg> args;
    std::unique_ptr<ScopeStmt> body;
    int line, col;
    Function(const std::string& n, std::vector<Arg> as, std::unique_ptr<ScopeStmt> b, Type rt, int l, int c)
        : name(n), return_type(rt), args(std::move(as)), body(std::move(b)), line(l), col(c) {}
    void print(int indent = 0) const;
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> globals;
    std::vector<std::unique_ptr<Function>> functions;
    void print(int indent = 0) const;
};

class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::unique_ptr<Program> parse_program();

private:
    std::vector<Token> m_tokens;
    size_t m_index = 0;

    std::optional<Token> peek(int offset = 0) const;
    Token consume();
    void report_error(const std::string& msg) const;

    Type parse_type();
    std::unique_ptr<Expr> parse_expr();
    std::unique_ptr<Expr> parse_assignment();
    std::unique_ptr<Expr> parse_logical_or();
    std::unique_ptr<Expr> parse_logical_and();
    std::unique_ptr<Expr> parse_comparison();
    std::unique_ptr<Expr> parse_additive();
    std::unique_ptr<Expr> parse_term();
    std::unique_ptr<Expr> parse_unary();
    std::unique_ptr<Expr> parse_factor();

    std::unique_ptr<Stmt> parse_stmt();
    std::unique_ptr<ScopeStmt> parse_scope();
    std::unique_ptr<Stmt> parse_struct_decl();
    std::unique_ptr<Stmt> parse_switch_stmt();
    std::unique_ptr<Stmt> parse_do_while_stmt();
    std::unique_ptr<Function> parse_function_rest(Type rt, std::string name, int l, int c);
};