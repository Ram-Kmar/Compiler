#pragma once
#include "lexer.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct Type {
  enum class Base { Int, Void, Bool } base;
  int ptr_level = 0; // 0 = value, 1 = *, 2 = **

  bool operator==(const Type& other) const {
    return base == other.base && ptr_level == other.ptr_level;
  }
  bool operator!=(const Type& other) const {
    return !(*this == other);
  }

  static Type Int() { return {Base::Int, 0}; }
  static Type Bool() { return {Base::Bool, 0}; }
  static Type Void() { return {Base::Void, 0}; }
};

struct IntLitExpr;
struct BoolLitExpr;
struct IdentifierExpr;
struct ArrayAccessExpr;
struct CallExpr;
struct UnaryExpr;
struct BinaryExpr;
struct ReturnStmt;
struct ExprStmt;
struct VarDecl;
struct AssignStmt;
struct ArrayAssignStmt;
struct PointerAssignStmt;
struct ScopeStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct Function;
struct Program;

class Visitor {
public:
  virtual ~Visitor() = default;
  virtual void visit(const IntLitExpr* node) = 0;
  virtual void visit(const BoolLitExpr* node) = 0;
  virtual void visit(const IdentifierExpr* node) = 0;
  virtual void visit(const ArrayAccessExpr* node) = 0;
  virtual void visit(const CallExpr* node) = 0;
  virtual void visit(const UnaryExpr* node) = 0;
  virtual void visit(const BinaryExpr* node) = 0;
  virtual void visit(const ReturnStmt* node) = 0;
  virtual void visit(const ExprStmt* node) = 0;
  virtual void visit(const VarDecl* node) = 0;
  virtual void visit(const AssignStmt* node) = 0;
  virtual void visit(const ArrayAssignStmt* node) = 0;
  virtual void visit(const PointerAssignStmt* node) = 0;
  virtual void visit(const ScopeStmt* node) = 0;
  virtual void visit(const IfStmt* node) = 0;
  virtual void visit(const WhileStmt* node) = 0;
  virtual void visit(const ForStmt* node) = 0;
  virtual void visit(const Function* node) = 0;
  virtual void visit(const Program* node) = 0;
};

struct Node {
  int line;
  int col;
  Node(int l, int c) : line(l), col(c) {}
  virtual ~Node() = default;
  virtual void print(int indent = 0) const = 0;
  virtual void accept(Visitor* visitor) const = 0;
};

struct Expr : public Node {
  using Node::Node;
};

struct IntLitExpr : public Expr {
  int value;
  IntLitExpr(int v, int l, int c) : Expr(l, c), value(v) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct BoolLitExpr : public Expr {
  bool value;
  BoolLitExpr(bool v, int l, int c) : Expr(l, c), value(v) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct IdentifierExpr : public Expr {
  std::string name;
  IdentifierExpr(std::string n, int l, int c) : Expr(l, c), name(std::move(n)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct ArrayAccessExpr : public Expr {
  std::string name;
  std::unique_ptr<Expr> index;
  ArrayAccessExpr(std::string n, std::unique_ptr<Expr> i, int l, int c)
      : Expr(l, c), name(std::move(n)), index(std::move(i)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct CallExpr : public Expr {
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
  CallExpr(std::string c, std::vector<std::unique_ptr<Expr>> a, int l, int c_col)
      : Expr(l, c_col), callee(std::move(c)), args(std::move(a)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct UnaryExpr : public Expr {
  std::unique_ptr<Expr> operand;
  TokenType op;
  UnaryExpr(std::unique_ptr<Expr> o, TokenType op, int l, int c)
      : Expr(l, c), operand(std::move(o)), op(op) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct BinaryExpr : public Expr {
  std::unique_ptr<Expr> lhs;
  std::unique_ptr<Expr> rhs;
  TokenType op;
  BinaryExpr(std::unique_ptr<Expr> l, std::unique_ptr<Expr> r, TokenType o, int line, int col)
      : Expr(line, col), lhs(std::move(l)), rhs(std::move(r)), op(o) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct Stmt : public Node {
  using Node::Node;
};

struct ReturnStmt : public Stmt {
  std::unique_ptr<Expr> expr;
  ReturnStmt(std::unique_ptr<Expr> e, int l, int c);
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct ExprStmt : public Stmt {
  std::unique_ptr<Expr> expr;
  ExprStmt(std::unique_ptr<Expr> e, int l, int c) : Stmt(l, c), expr(std::move(e)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct VarDecl : public Stmt {
  std::string name;
  Type type;
  std::unique_ptr<Expr> init;
  std::optional<int> array_size;
  VarDecl(std::string n, Type t, std::unique_ptr<Expr> i, int l, int c, std::optional<int> as = std::nullopt)
      : Stmt(l, c), name(std::move(n)), type(t), init(std::move(i)), array_size(as) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct AssignStmt : public Stmt {
  std::string name;
  std::unique_ptr<Expr> value;
  AssignStmt(std::string n, std::unique_ptr<Expr> v, int l, int c)
      : Stmt(l, c), name(std::move(n)), value(std::move(v)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct ArrayAssignStmt : public Stmt {
  std::string name;
  std::unique_ptr<Expr> index;
  std::unique_ptr<Expr> value;
  ArrayAssignStmt(std::string n, std::unique_ptr<Expr> i, std::unique_ptr<Expr> v, int l, int c)
      : Stmt(l, c), name(std::move(n)), index(std::move(i)), value(std::move(v)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct PointerAssignStmt : public Stmt {
  std::unique_ptr<Expr> ptr_expr; // The expression evaluating to the pointer (e.g., p, or *pp)
  std::unique_ptr<Expr> value;
  PointerAssignStmt(std::unique_ptr<Expr> p, std::unique_ptr<Expr> v, int l, int c)
      : Stmt(l, c), ptr_expr(std::move(p)), value(std::move(v)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct ScopeStmt : public Stmt {
  std::vector<std::unique_ptr<Stmt>> stmts;
  ScopeStmt(std::vector<std::unique_ptr<Stmt>> s, int l, int c) : Stmt(l, c), stmts(std::move(s)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct IfStmt : public Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> then_stmt;
  std::unique_ptr<Stmt> else_stmt;
  IfStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> t,
         std::unique_ptr<Stmt> e = nullptr, int l = 0, int c_col = 0)
      : Stmt(l, c_col), condition(std::move(c)), then_stmt(std::move(t)),
        else_stmt(std::move(e)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct WhileStmt : public Stmt {
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> body;
  WhileStmt(std::unique_ptr<Expr> c, std::unique_ptr<Stmt> b, int l, int c_col)
      : Stmt(l, c_col), condition(std::move(c)), body(std::move(b)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct ForStmt : public Stmt {
  std::unique_ptr<Stmt> init;
  std::unique_ptr<Expr> condition;
  std::unique_ptr<Stmt> increment;
  std::unique_ptr<Stmt> body;
  ForStmt(std::unique_ptr<Stmt> i, std::unique_ptr<Expr> c, std::unique_ptr<Stmt> inc, std::unique_ptr<Stmt> b, int l, int c_col)
      : Stmt(l, c_col), init(std::move(i)), condition(std::move(c)), increment(std::move(inc)), body(std::move(b)) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct Arg {
  std::string name;
  Type type;
};

struct Function : public Node {
  std::string name;
  std::vector<Arg> args;
  std::unique_ptr<ScopeStmt> body;
  Type return_type;

  Function(std::string n, std::vector<Arg> a, std::unique_ptr<ScopeStmt> b,
           Type rt, int l, int c)
      : Node(l, c), name(std::move(n)), args(std::move(a)), body(std::move(b)),
        return_type(rt) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
};

struct Program : public Node {
  std::vector<std::unique_ptr<Function>> functions;
  std::vector<std::unique_ptr<Stmt>> globals;
  Program() : Node(1, 1) {}
  void print(int indent = 0) const override;
  void accept(Visitor* visitor) const override { visitor->visit(this); }
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
  void report_error(const std::string& message, std::optional<Token> token = std::nullopt) const;

  std::unique_ptr<Function> parse_function();
  std::unique_ptr<Stmt> parse_stmt();
  std::unique_ptr<Stmt> parse_scope();

  std::unique_ptr<Expr> parse_expr();
  std::unique_ptr<Expr> parse_logical_or();
  std::unique_ptr<Expr> parse_logical_and();
  std::unique_ptr<Expr> parse_comparison();
  std::unique_ptr<Expr> parse_additive();
  std::unique_ptr<Expr> parse_term();
  std::unique_ptr<Expr> parse_unary();
  std::unique_ptr<Expr> parse_factor();
};
