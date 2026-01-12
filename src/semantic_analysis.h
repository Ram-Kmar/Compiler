#pragma once
#include "parser.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <optional>

struct Symbol {
    Type type;
    std::optional<int> array_size;
};

class SemanticAnalyzer {
public:
    explicit SemanticAnalyzer(Program* program);
    void analyze();

private:
    Program* m_prog;
    
    // Scopes: Stack of maps from name to Symbol
    std::vector<std::unordered_map<std::string, Symbol>> m_scopes;
    
    // Function signatures
    struct FuncSignature {
        Type return_type;
        std::vector<Type> arg_types;
    };
    std::unordered_map<std::string, FuncSignature> m_functions;
    
    // Current context
    std::optional<Type> m_current_func_return_type;

    void push_scope();
    void pop_scope();
    void declare_var(const std::string& name, Type type, const Node* node, std::optional<int> array_size = std::nullopt);
    std::optional<Symbol> find_var(const std::string& name);

    void analyze_stmt(const Stmt* stmt);
    Type analyze_expr(const Expr* expr);
    
    void register_function(const Function* func);
    void analyze_function(const Function* func);

    void report_error(const std::string& message, const Node* node);

    // Visitor implementation (needed if we want to use visitor for semantic analysis, 
    // but current implementation uses dynamic_cast. Let's stay consistent for now 
    // or refactor this too. Let's stay with dynamic_cast for semantic for speed of implementation now,
    // though Visitor is better.)
};
