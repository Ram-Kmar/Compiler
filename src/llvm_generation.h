#pragma once
#include "parser.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

struct LLVMVarInfo {
    std::string name;
    Type type;
};

class LLVMGenerator : public Visitor {
public:
    explicit LLVMGenerator(const Program* root);
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
    void visit(const Program* node) override;

private:
    const Program* m_root;
    std::stringstream m_output;
    int m_reg_count = 0;
    int m_label_count = 0;
    
    // Stack of scopes. Each scope is a map of variable names to their LLVM register name.
    std::vector<std::unordered_map<std::string, LLVMVarInfo>> m_scopes;
    
    // Last generated register
    std::string m_last_reg;

    // Helpers
    std::string new_reg();
    std::string new_label();
    void push_scope();
    void pop_scope();
    void declare_var(const std::string& name, Type type);
    std::optional<LLVMVarInfo> find_var(const std::string& name);
    std::string to_llvm_type(Type type);
};
