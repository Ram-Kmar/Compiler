#pragma once
#include "parser.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

struct VarInfo {
    size_t stack_offset;
};

class Generator : public Visitor {
public:
    explicit Generator(const Program* root);
    std::string generate();

    // Visitor Implementation
    void visit(const IntLitExpr* node) override;
    void visit(const BoolLitExpr* node) override;
    void visit(const IdentifierExpr* node) override;
    void visit(const ArrayAccessExpr* node) override;
    void visit(const CallExpr* node) override;
    void visit(const UnaryExpr* node) override;
    void visit(const BinaryExpr* node) override;
    void visit(const ReturnStmt* node) override;
    void visit(const ExprStmt* node) override;
    void visit(const VarDecl* node) override;
    void visit(const AssignStmt* node) override;
    void visit(const ArrayAssignStmt* node) override;
    void visit(const PointerAssignStmt* node) override;
    void visit(const ScopeStmt* node) override;
    void visit(const IfStmt* node) override;
    void visit(const WhileStmt* node) override;
    void visit(const ForStmt* node) override;
    void visit(const Function* node) override;
    void visit(const Layer* node) override;
    void visit(const Program* node) override;

private:
    const Program* m_root;
    std::stringstream m_output;
    size_t m_stack_ptr = 0;
    int m_label_count = 0;
    
    // Stack of scopes. Each scope is a map of variable names to their info.
    std::vector<std::unordered_map<std::string, VarInfo>> m_scopes;
    
    // Helpers
    std::string create_label();
    void push_scope();
    void pop_scope();
    void declare_var(const std::string& name, std::optional<int> array_size = std::nullopt);
    std::optional<VarInfo> find_var(const std::string& name);
};
