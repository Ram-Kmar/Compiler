#pragma once
#include "parser.h"
#include <string>
#include <sstream>
#include <unordered_map>

struct VarInfo {
    size_t stack_offset;
};

class Generator {
public:
    explicit Generator(std::unique_ptr<Program> root);
    std::string generate();

private:
    std::unique_ptr<Program> m_root;
    std::stringstream m_output;
    std::unordered_map<std::string, VarInfo> m_vars;
    size_t m_stack_ptr = 0;
    
    void gen_stmt(const Stmt* stmt);
    void gen_expr(const Expr* expr);
};