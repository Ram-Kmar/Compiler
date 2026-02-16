#pragma once
#include "parser.h"
#include <memory>
#include <vector>

class Optimizer : public Visitor {
public:
    std::unique_ptr<Program> optimize(std::unique_ptr<Program> program);

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
    // void visit(const Layer* node) override;
    void visit(const Program* node) override;

private:
    std::unique_ptr<Node> m_last_node;

    template<typename T>
    std::unique_ptr<T> transform(const T* node) {
        if (!node) return nullptr;
        node->accept(this);
        auto result = std::unique_ptr<T>(static_cast<T*>(m_last_node.release()));
        return result;
    }

    std::unique_ptr<Expr> transform_expr(const Expr* node) {
        if (!node) return nullptr;
        node->accept(this);
        auto result = std::unique_ptr<Expr>(static_cast<Expr*>(m_last_node.release()));
        return result;
    }

    std::unique_ptr<Stmt> transform_stmt(const Stmt* node) {
        if (!node) return nullptr;
        node->accept(this);
        auto result = std::unique_ptr<Stmt>(static_cast<Stmt*>(m_last_node.release()));
        return result;
    }
};
