#include "optimizer.h"

std::unique_ptr<Program> Optimizer::optimize(std::unique_ptr<Program> program) {
    if (!program) return nullptr;
    program->accept(this);
    return std::unique_ptr<Program>(static_cast<Program*>(m_last_node.release()));
}

void Optimizer::visit(const IntLitExpr* node) {
    m_last_node = std::make_unique<IntLitExpr>(node->value, node->line, node->col);
}

void Optimizer::visit(const BoolLitExpr* node) {
    m_last_node = std::make_unique<BoolLitExpr>(node->value, node->line, node->col);
}

void Optimizer::visit(const IdentifierExpr* node) {
    m_last_node = std::make_unique<IdentifierExpr>(node->name, node->line, node->col);
}

void Optimizer::visit(const ArrayAccessExpr* node) {
    m_last_node = std::make_unique<ArrayAccessExpr>(node->name, transform_expr(node->index.get()), node->line, node->col);
}

void Optimizer::visit(const CallExpr* node) {
    std::vector<std::unique_ptr<Expr>> folded_args;
    for (const auto& arg : node->args) {
        folded_args.push_back(transform_expr(arg.get()));
    }
    m_last_node = std::make_unique<CallExpr>(node->callee, std::move(folded_args), node->line, node->col);
}

void Optimizer::visit(const UnaryExpr* node) {
    auto operand = transform_expr(node->operand.get());
    
    // Constant folding for !
    if (node->op == TokenType::bang) {
        if (auto* bool_lit = dynamic_cast<BoolLitExpr*>(operand.get())) {
            m_last_node = std::make_unique<BoolLitExpr>(!bool_lit->value, node->line, node->col);
            return;
        }
    }
    
    m_last_node = std::make_unique<UnaryExpr>(std::move(operand), node->op, node->line, node->col);
}

void Optimizer::visit(const BinaryExpr* node) {
    auto lhs = transform_expr(node->lhs.get());
    auto rhs = transform_expr(node->rhs.get());
    
    auto* l_int = dynamic_cast<IntLitExpr*>(lhs.get());
    auto* r_int = dynamic_cast<IntLitExpr*>(rhs.get());
    
    if (l_int && r_int) {
        int v1 = l_int->value;
        int v2 = r_int->value;
        if (node->op == TokenType::plus) {
            m_last_node = std::make_unique<IntLitExpr>(v1 + v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::minus) {
            m_last_node = std::make_unique<IntLitExpr>(v1 - v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::star) {
            m_last_node = std::make_unique<IntLitExpr>(v1 * v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::slash && v2 != 0) {
            m_last_node = std::make_unique<IntLitExpr>(v1 / v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::eq_eq) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 == v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::neq) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 != v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::lt) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 < v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::gt) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 > v2, node->line, node->col);
            return;
        }
    }
    
    auto* l_bool = dynamic_cast<BoolLitExpr*>(lhs.get());
    auto* r_bool = dynamic_cast<BoolLitExpr*>(rhs.get());
    if (l_bool && r_bool) {
        bool v1 = l_bool->value;
        bool v2 = r_bool->value;
        if (node->op == TokenType::amp_amp) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 && v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::pipe_pipe) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 || v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::eq_eq) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 == v2, node->line, node->col);
            return;
        } else if (node->op == TokenType::neq) {
            m_last_node = std::make_unique<BoolLitExpr>(v1 != v2, node->line, node->col);
            return;
        }
    }
    
    m_last_node = std::make_unique<BinaryExpr>(std::move(lhs), std::move(rhs), node->op, node->line, node->col);
}

void Optimizer::visit(const ReturnStmt* node) {
    m_last_node = std::make_unique<ReturnStmt>(transform_expr(node->expr.get()), node->line, node->col);
}

void Optimizer::visit(const ExprStmt* node) {
    m_last_node = std::make_unique<ExprStmt>(transform_expr(node->expr.get()), node->line, node->col);
}

void Optimizer::visit(const VarDecl* node) {
    m_last_node = std::make_unique<VarDecl>(node->name, node->type, 
        node->init ? transform_expr(node->init.get()) : nullptr, 
        node->line, node->col, node->array_size);
}

void Optimizer::visit(const AssignStmt* node) {
    m_last_node = std::make_unique<AssignStmt>(node->name, transform_expr(node->value.get()), node->line, node->col);
}

void Optimizer::visit(const ArrayAssignStmt* node) {
    m_last_node = std::make_unique<ArrayAssignStmt>(node->name, 
        transform_expr(node->index.get()), 
        transform_expr(node->value.get()), 
        node->line, node->col);
}

void Optimizer::visit(const PointerAssignStmt* node) {
    m_last_node = std::make_unique<PointerAssignStmt>(
        transform_expr(node->ptr_expr.get()), 
        transform_expr(node->value.get()), 
        node->line, node->col);
}

void Optimizer::visit(const ScopeStmt* node) {
    std::vector<std::unique_ptr<Stmt>> folded_stmts;
    for (const auto& s : node->stmts) {
        folded_stmts.push_back(transform_stmt(s.get()));
    }
    m_last_node = std::make_unique<ScopeStmt>(std::move(folded_stmts), node->line, node->col);
}

void Optimizer::visit(const IfStmt* node) {
    m_last_node = std::make_unique<IfStmt>(
        transform_expr(node->condition.get()),
        transform_stmt(node->then_stmt.get()),
        node->else_stmt ? transform_stmt(node->else_stmt.get()) : nullptr,
        node->line, node->col
    );
}

void Optimizer::visit(const WhileStmt* node) {
    m_last_node = std::make_unique<WhileStmt>(
        transform_expr(node->condition.get()),
        transform_stmt(node->body.get()),
        node->line, node->col
    );
}

void Optimizer::visit(const ForStmt* node) {
    m_last_node = std::make_unique<ForStmt>(
        node->init ? transform_stmt(node->init.get()) : nullptr,
        node->condition ? transform_expr(node->condition.get()) : nullptr,
        node->increment ? transform_stmt(node->increment.get()) : nullptr,
        transform_stmt(node->body.get()),
        node->line, node->col
    );
}

void Optimizer::visit(const Function* node) {
    auto body = std::unique_ptr<ScopeStmt>(static_cast<ScopeStmt*>(transform_stmt(node->body.get()).release()));
    m_last_node = std::make_unique<Function>(node->name, node->args, std::move(body), node->return_type, node->line, node->col);
}

void Optimizer::visit(const Program* node) {
    auto folded_prog = std::make_unique<Program>();
    for (const auto& func : node->functions) {
        folded_prog->functions.push_back(std::unique_ptr<Function>(static_cast<Function*>(transform(func.get()).release())));
    }
    for (const auto& global : node->globals) {
        folded_prog->globals.push_back(transform_stmt(global.get()));
    }
    m_last_node = std::move(folded_prog);
}
