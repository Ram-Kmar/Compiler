#include "generation.h"
#include <iostream>

Generator::Generator(std::unique_ptr<Program> root) : m_root(std::move(root)) {}

std::string Generator::generate() {
    m_output << ".global _main\n";
    m_output << ".align 2\n\n";
    m_output << "_main:\n";

    // Prologue: Save frame pointer and link register
    m_output << "    stp x29, x30, [sp, #-16]!\n";
    m_output << "    mov x29, sp\n";

    for (const auto& stmt : m_root->stmts) {
        gen_stmt(stmt.get());
    }

    // Default return 0 if no return statement
    m_output << "    mov x0, #0\n";
    
    // Epilogue: Restore fp and lr
    m_output << "    ldp x29, x30, [sp], #16\n";
    m_output << "    ret\n";

    return m_output.str();
}

void Generator::gen_stmt(const Stmt* stmt) {
    if (const auto* ret_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        gen_expr(ret_stmt->expr.get());
        // Epilogue before return
        m_output << "    mov sp, x29\n";
        m_output << "    ldp x29, x30, [sp], #16\n";
        m_output << "    ret\n";
    } else if (const auto* var_decl = dynamic_cast<const VarDecl*>(stmt)) {
        if (m_vars.count(var_decl->name)) {
            std::cerr << "Error: Variable already declared: " << var_decl->name << std::endl;
            exit(1);
        }
        gen_expr(var_decl->init.get());
        
        // Push to stack
        m_output << "    str x0, [sp, #-16]!\n";
        m_stack_ptr += 16;
        m_vars[var_decl->name] = { m_stack_ptr };
    }
}

void Generator::gen_expr(const Expr* expr) {
    if (const auto* int_lit = dynamic_cast<const IntLitExpr*>(expr)) {
        m_output << "    mov x0, #" << int_lit->value << "\n";
    } else if (const auto* ident_expr = dynamic_cast<const IdentifierExpr*>(expr)) {
        if (m_vars.count(ident_expr->name) == 0) {
            std::cerr << "Error: Undeclared variable: " << ident_expr->name << std::endl;
            exit(1);
        }
        const auto& var = m_vars.at(ident_expr->name);
        // Calculate offset from x29 (fp)
        // Since we push 16 bytes at a time, and x29 points to the old x29/x30...
        // The first variable is at [x29, -16]
        int offset = -(int)var.stack_offset;
        m_output << "    ldr x0, [x29, #" << offset << "]\n";
    }
}