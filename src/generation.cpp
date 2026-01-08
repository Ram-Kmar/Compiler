#include "generation.h"
#include <iostream>

Generator::Generator(std::unique_ptr<Program> root) : m_root(std::move(root)) {
    // Global scope
    m_scopes.push_back({}); 
}

std::string Generator::create_label() {
    return ".L" + std::to_string(m_label_count++);
}

void Generator::push_scope() {
    m_scopes.push_back({});
}

void Generator::pop_scope() {
    // Variables in the current scope are removed from lookup map
    // Note: We handle the actual stack pointer adjustment inside gen_stmt(ScopeStmt) 
    // because we need to know the byte count there.
    m_scopes.pop_back();
}

void Generator::declare_var(const std::string& name) {
    if (m_scopes.back().count(name)) {
        std::cerr << "Error: Variable '" << name << "' already declared in this scope." << std::endl;
        exit(1);
    }
    // Stack grows down, so add 16 bytes
    m_stack_ptr += 16;
    m_scopes.back()[name] = { m_stack_ptr };
}

std::optional<VarInfo> Generator::find_var(const std::string& name) {
    // Search from inner-most scope to outer-most
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        if (it->count(name)) {
            return it->at(name);
        }
    }
    return std::nullopt;
}

std::string Generator::generate() {
    m_output << ".global _main\n";
    m_output << ".align 2\n\n";
    m_output << "_main:\n";

    // Prologue
    m_output << "    stp x29, x30, [sp, #-16]!\n";
    m_output << "    mov x29, sp\n";

    for (const auto& stmt : m_root->stmts) {
        gen_stmt(stmt.get());
    }

    // Default return 0
    m_output << "    mov x0, #0\n";
    
    // Epilogue
    m_output << "    mov sp, x29\n"; // Restore stack pointer just in case
    m_output << "    ldp x29, x30, [sp], #16\n";
    m_output << "    ret\n";

    return m_output.str();
}

void Generator::gen_stmt(const Stmt* stmt) {
    if (const auto* ret_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        gen_expr(ret_stmt->expr.get());
        m_output << "    mov sp, x29\n"; // Reset stack before returning
        m_output << "    ldp x29, x30, [sp], #16\n";
        m_output << "    ret\n";
    } 
    else if (const auto* var_decl = dynamic_cast<const VarDecl*>(stmt)) {
        gen_expr(var_decl->init.get());
        m_output << "    str x0, [sp, #-16]!\n"; // Push to stack
        declare_var(var_decl->name);
    }
    else if (const auto* assign_stmt = dynamic_cast<const AssignStmt*>(stmt)) {
        auto var = find_var(assign_stmt->name);
        if (!var.has_value()) {
            std::cerr << "Error: Undeclared variable: " << assign_stmt->name << std::endl;
            exit(1);
        }
        gen_expr(assign_stmt->value.get());
        int offset = -(int)var->stack_offset;
        m_output << "    str x0, [x29, #" << offset << "]\n";
    }
    else if (const auto* scope_stmt = dynamic_cast<const ScopeStmt*>(stmt)) {
        size_t saved_stack_ptr = m_stack_ptr;
        push_scope();
        
        for (const auto& s : scope_stmt->stmts) {
            gen_stmt(s.get());
        }
        
        // Cleanup stack
        size_t bytes_to_pop = m_stack_ptr - saved_stack_ptr;
        if (bytes_to_pop > 0) {
            m_output << "    add sp, sp, #" << bytes_to_pop << "\n";
        }
        m_stack_ptr = saved_stack_ptr; // Reset internal counter
        
        pop_scope();
    }
    else if (const auto* if_stmt = dynamic_cast<const IfStmt*>(stmt)) {
        std::string label_else = create_label();
        std::string label_end = create_label();
        
        gen_expr(if_stmt->condition.get());
        m_output << "    cmp x0, #0\n";
        m_output << "    b.eq " << label_else << "\n"; // Jump to else if false (0)
        
        gen_stmt(if_stmt->then_stmt.get());
        m_output << "    b " << label_end << "\n";
        
        m_output << label_else << ":\n";
        if (if_stmt->else_stmt) {
            gen_stmt(if_stmt->else_stmt.get());
        }
        
        m_output << label_end << ":\n";
    }
    else if (const auto* while_stmt = dynamic_cast<const WhileStmt*>(stmt)) {
        std::string label_start = create_label();
        std::string label_end = create_label();
        
        m_output << label_start << ":\n";
        gen_expr(while_stmt->condition.get());
        m_output << "    cmp x0, #0\n";
        m_output << "    b.eq " << label_end << "\n";
        
        gen_stmt(while_stmt->body.get());
        m_output << "    b " << label_start << "\n";
        
        m_output << label_end << ":\n";
    }
}

void Generator::gen_expr(const Expr* expr) {
    if (const auto* int_lit = dynamic_cast<const IntLitExpr*>(expr)) {
        m_output << "    mov x0, #" << int_lit->value << "\n";
    } else if (const auto* ident_expr = dynamic_cast<const IdentifierExpr*>(expr)) {
        auto var = find_var(ident_expr->name);
        if (!var.has_value()) {
            std::cerr << "Error: Undeclared variable: " << ident_expr->name << std::endl;
            exit(1);
        }
        // Offset is negative relative to x29
        int offset = -(int)var->stack_offset;
        m_output << "    ldr x0, [x29, #" << offset << "]\n";
    } else if (const auto* bin_expr = dynamic_cast<const BinaryExpr*>(expr)) {
        gen_expr(bin_expr->rhs.get());
        m_output << "    str x0, [sp, #-16]!\n";
        
        gen_expr(bin_expr->lhs.get());
        m_output << "    ldr x1, [sp], #16\n";
        
        if (bin_expr->op == TokenType::plus) {
            m_output << "    add x0, x0, x1\n";
        } else if (bin_expr->op == TokenType::minus) {
            m_output << "    sub x0, x0, x1\n";
        } else if (bin_expr->op == TokenType::star) {
            m_output << "    mul x0, x0, x1\n";
        } else if (bin_expr->op == TokenType::slash) {
            m_output << "    sdiv x0, x0, x1\n";
        }
    }
}