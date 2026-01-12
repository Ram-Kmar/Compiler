#pragma once
#include "parser.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

struct VarInfo {
    size_t stack_offset;
};

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
    size_t m_stack_ptr = 0;
    int m_label_count = 0;
    
    // Stack of scopes. Each scope is a map of variable names to their info.
    std::vector<std::unordered_map<std::string, VarInfo>> m_scopes;
    
    void gen_function(const Function* func);
    void gen_stmt(const Stmt* stmt);
    void gen_expr(const Expr* expr);
    
    // Helpers
    std::string create_label();
    void push_scope();
    void pop_scope();
    void declare_var(const std::string& name);
    std::optional<VarInfo> find_var(const std::string& name);
};