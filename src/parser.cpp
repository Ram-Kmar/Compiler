// parser.cpp
#include "parser.h"
#include <iostream>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

std::optional<Token> Parser::peek(int offset) const {
    if (m_index + offset >= m_tokens.size()) return std::nullopt;
    return m_tokens[m_index + offset];
}

Token Parser::consume() { return m_tokens[m_index++]; }

void Parser::report_error(const std::string& msg) const {
    auto t = peek();
    if (t) std::cerr << "Error at " << t->line << ":" << t->col << " - " << msg << "\n";
    else std::cerr << "Error at EOF - " << msg << "\n";
    exit(1);
}

Type Parser::parse_type() {
    Type t;
    auto tok = peek();
    if (tok->type == TokenType::_int) { consume(); t = Type::Int(); }
    else if (tok->type == TokenType::_char) { consume(); t = Type::Char(); }
    else if (tok->type == TokenType::_float) { consume(); t = Type::Float(); }
    else if (tok->type == TokenType::_double) { consume(); t = Type::Double(); }
    else if (tok->type == TokenType::_void) { consume(); t.base = Type::Base::Void; }
    else if (tok->type == TokenType::_struct) {
        consume();
        t = Type::Struct(std::get<std::string>(consume().value));
    }
    while (peek() && peek()->type == TokenType::star) { consume(); t.ptr_level++; }
    return t;
}

std::unique_ptr<Expr> Parser::parse_expr() { return parse_assignment(); }

std::unique_ptr<Expr> Parser::parse_assignment() {
    auto left = parse_logical_or();
    if (peek() && (peek()->type == TokenType::eq || peek()->type == TokenType::plus_eq || peek()->type == TokenType::minus_eq)) {
        Token op = consume();
        auto right = parse_assignment();
        return std::make_unique<BinaryExpr>(std::move(left), std::move(right), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_logical_or() {
    auto left = parse_logical_and();
    while (peek() && peek()->type == TokenType::pipe_pipe) {
        Token op = consume();
        left = std::make_unique<BinaryExpr>(std::move(left), parse_logical_and(), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_logical_and() {
    auto left = parse_comparison();
    while (peek() && peek()->type == TokenType::amp_amp) {
        Token op = consume();
        left = std::make_unique<BinaryExpr>(std::move(left), parse_comparison(), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_comparison() {
    auto left = parse_additive();
    while (peek() && (peek()->type == TokenType::eq_eq || peek()->type == TokenType::neq || peek()->type == TokenType::lt || peek()->type == TokenType::gt)) {
        Token op = consume();
        left = std::make_unique<BinaryExpr>(std::move(left), parse_additive(), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_additive() {
    auto left = parse_term();
    while (peek() && (peek()->type == TokenType::plus || peek()->type == TokenType::minus)) {
        Token op = consume();
        left = std::make_unique<BinaryExpr>(std::move(left), parse_term(), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_term() {
    auto left = parse_unary();
    while (peek() && (peek()->type == TokenType::star || peek()->type == TokenType::slash)) {
        Token op = consume();
        left = std::make_unique<BinaryExpr>(std::move(left), parse_unary(), op.type, op.line, op.col);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parse_unary() {
    if (peek() && (peek()->type == TokenType::bang || peek()->type == TokenType::minus || peek()->type == TokenType::star || peek()->type == TokenType::amp)) {
        Token op = consume();
        return std::make_unique<BinaryExpr>(nullptr, parse_unary(), op.type, op.line, op.col); // Simplified Unary as Binary with null LHS for this scope
    }
    return parse_factor();
}

std::unique_ptr<Expr> Parser::parse_factor() {
    auto t = consume();
    if (t.type == TokenType::int_lit) return std::make_unique<IntLitExpr>(std::get<int>(t.value), t.line, t.col);
    if (t.type == TokenType::string_lit) return std::make_unique<StringLitExpr>(std::get<std::string>(t.value), t.line, t.col);
    if (t.type == TokenType::ident) {
        if (peek() && peek()->type == TokenType::open_paren) {
            consume();
            std::vector<std::unique_ptr<Expr>> args;
            if (peek()->type != TokenType::close_paren) {
                //do { args.push_back(parse_expr()); } while (peek()->type == TokenType::comma && consume().type);
                /*while (peek()->type == TokenType::comma) {
                    consume();
                    args.push_back(parse_expr());
                }*/
                do {
                    args.push_back(parse_expr());
                    if (peek()->type == TokenType::comma) {
                        consume();   // eat ','
                    }
                    else {
                        break;
                    }
                } while (true);

            }
            consume(); // )
            return std::make_unique<CallExpr>(std::get<std::string>(t.value), std::move(args), t.line, t.col);
        }
        return std::make_unique<IdentifierExpr>(std::get<std::string>(t.value), t.line, t.col);
    }
    if (t.type == TokenType::open_paren) {
        auto e = parse_expr();
        consume(); // )
        return e;
    }
    report_error("Invalid expression");
    return nullptr;
}

std::unique_ptr<Stmt> Parser::parse_stmt() {
    auto t = peek();
    std::cout << "[DEBUG Parser] Parsing statement starting with token at " << t->line << ":" << t->col << std::endl;
    if (t->type == TokenType::_return) {
        consume();
        auto e = parse_expr();
        consume(); // ;
        return std::make_unique<ReturnStmt>(std::move(e), t->line, t->col);
    }
    if (t->type == TokenType::_if) {
        consume(); consume(); // (
        auto c = parse_expr(); consume(); // )
        auto ts = parse_stmt();
        std::unique_ptr<Stmt> es = nullptr;
        if (peek() && peek()->type == TokenType::_else) { consume(); es = parse_stmt(); }
        return std::make_unique<IfStmt>(std::move(c), std::move(ts), std::move(es), t->line, t->col);
    }
    if (t->type == TokenType::_while) {
        consume(); consume(); // (
        auto c = parse_expr(); consume(); // )
        return std::make_unique<WhileStmt>(std::move(c), parse_stmt(), t->line, t->col);
    }
    if (t->type == TokenType::open_curly) return parse_scope();
    
    // Check for declaration
    bool is_type = (t->type == TokenType::_int || t->type == TokenType::_char || t->type == TokenType::_float || t->type == TokenType::_double || t->type == TokenType::_void || t->type == TokenType::_struct);
    if (is_type) {
        auto type = parse_type();
        auto name = consume();
        std::unique_ptr<Expr> init = nullptr;
        if (peek()->type == TokenType::eq) { consume(); init = parse_expr(); }
        consume(); // ;
        return std::make_unique<VarDecl>(std::get<std::string>(name.value), type, std::move(init), name.line, name.col);
    }

    auto e = parse_expr();
    consume(); // ;
    return std::make_unique<ExprStmt>(std::move(e), t->line, t->col);
}

std::unique_ptr<ScopeStmt> Parser::parse_scope() {
    auto t = consume(); // {
    std::vector<std::unique_ptr<Stmt>> ss;
    while (peek() && peek()->type != TokenType::close_curly) ss.push_back(parse_stmt());
    consume(); // }
    return std::make_unique<ScopeStmt>(std::move(ss), t.line, t.col);
}

std::unique_ptr<Function> Parser::parse_function_rest(Type rt, std::string name, int l, int c) {
    std::cout << "[DEBUG Parser] Entering function body: " << name << std::endl;
    consume(); // (
    std::vector<Arg> args;
    if (peek()->type != TokenType::close_paren) {
        do {
            auto type = parse_type();
            auto aname = consume();
            args.push_back({std::get<std::string>(aname.value), type});
        } while (peek()->type == TokenType::comma && (consume(), true));
    }
    consume(); // )
    return std::make_unique<Function>(name, args, parse_scope(), rt, l, c);
}

std::unique_ptr<Program> Parser::parse_program() {
    auto p = std::make_unique<Program>();
    while (peek() && peek()->type != TokenType::eof) {
        auto type = parse_type();
        auto name = consume();
        if (peek() && peek()->type == TokenType::open_paren) {
            p->functions.push_back(parse_function_rest(type, std::get<std::string>(name.value), name.line, name.col));
        } else {
            std::unique_ptr<Expr> init = nullptr;
            if (peek()->type == TokenType::eq) { consume(); init = parse_expr(); }
            consume(); // ;
            p->globals.push_back(std::make_unique<VarDecl>(std::get<std::string>(name.value), type, std::move(init), name.line, name.col));
        }
    }
    return p;
}

void print_indent(int i) { while(i--) std::cout << "  "; }
void IntLitExpr::print(int i) const { print_indent(i); std::cout << "Int(" << value << ")\n"; }
void StringLitExpr::print(int i) const { print_indent(i); std::cout << "Str(\"" << value << "\")\n"; }
void IdentifierExpr::print(int i) const { print_indent(i); std::cout << "Id(" << name << ")\n"; }
void BinaryExpr::print(int i) const { print_indent(i); std::cout << "Binary\n"; if(lhs) lhs->print(i+1); if(rhs) rhs->print(i+1); }
void CallExpr::print(int i) const { print_indent(i); std::cout << "Call(" << callee << ")\n"; for(auto& a: args) a->print(i+1); }
void ReturnStmt::print(int i) const { print_indent(i); std::cout << "Return\n"; expr->print(i+1); }
void VarDecl::print(int i) const { print_indent(i); std::cout << "Var(" << name << ")\n"; if(init) init->print(i+1); }
void ScopeStmt::print(int i) const { print_indent(i); std::cout << "Scope\n"; for(auto& s: stmts) s->print(i+1); }
void IfStmt::print(int i) const { print_indent(i); std::cout << "If\n"; condition->print(i+1); then_stmt->print(i+1); if(else_stmt) else_stmt->print(i+1); }
void WhileStmt::print(int i) const { print_indent(i); std::cout << "While\n"; condition->print(i+1); body->print(i+1); }
void ForStmt::print(int i) const { print_indent(i); std::cout << "For\n"; body->print(i+1); }
void DoWhileStmt::print(int i) const { print_indent(i); std::cout << "DoWhile\n"; body->print(i+1); condition->print(i+1); }
void SwitchStmt::print(int i) const { print_indent(i); std::cout << "Switch\n"; }
void StructDecl::print(int i) const { print_indent(i); std::cout << "Struct(" << name << ")\n"; }
void ExprStmt::print(int i) const { print_indent(i); std::cout << "ExprStmt\n"; expr->print(i+1); }
void Function::print(int i) const { print_indent(i); std::cout << "Func(" << name << ")\n"; body->print(i+1); }
void Program::print(int i) const { std::cout << "Program\n"; for(auto& g: globals) g->print(i+1); for(auto& f: functions) f->print(i+1); }
