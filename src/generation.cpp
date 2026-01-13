#include "generation.h"
#include <iostream>

Generator::Generator(const Program* root) : m_root(root) {
}

std::string Generator::create_label() {
    return ".L" + std::to_string(m_label_count++);
}

void Generator::push_scope() {
    m_scopes.push_back({});
}

void Generator::pop_scope() {
    m_scopes.pop_back();
}

void Generator::declare_var(const std::string& name, std::optional<int> array_size) {
    if (m_scopes.back().count(name)) {
        std::cerr << "Error: Variable '" << name << "' already declared in this scope." << std::endl;
        exit(1);
    }
    size_t size = array_size.has_value() ? (*array_size * 16) : 16;
    m_stack_ptr += size;
    m_scopes.back()[name] = { m_stack_ptr };
}

std::optional<VarInfo> Generator::find_var(const std::string& name) {
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        if (it->count(name)) {
            return it->at(name);
        }
    }
    return std::nullopt;
}

std::string Generator::generate() {
    m_root->accept(this);
    return m_output.str();
}

void Generator::visit(const Program* node) {
    m_output << ".global _main\n";
    m_output << ".align 2\n\n";
    m_output << ".data\n";
    m_output << "fmt: .asciz \"%d\n\"\n";
    m_output << ".text\n\n";

    bool has_main = false;
    for (const auto& func : node->functions) {
        if (func->name == "main") has_main = true;
        func->accept(this);
    }

    if (!has_main && !node->globals.empty()) {
        m_output << "_main:\n";
        m_output << "    stp x29, x30, [sp, #-16]!\n";
        m_output << "    mov x29, sp\n";
        
        m_stack_ptr = 0;
        push_scope();
        for (const auto& stmt : node->globals) {
            stmt->accept(this);
        }
        m_output << "    mov x0, #0\n";
        m_output << "    mov sp, x29\n";
        m_output << "    ldp x29, x30, [sp], #16\n";
        m_output << "    ret\n\n";
        pop_scope();
    }
}

void Generator::visit(const Function* node) {
    m_output << "_" << node->name << ":\n";
    m_output << "    stp x29, x30, [sp, #-16]!\n";
    m_output << "    mov x29, sp\n";
    
    m_stack_ptr = 0; 
    push_scope(); 
    
    for (size_t i = 0; i < node->args.size(); ++i) {
        if (i < 8) {
            m_output << "    str x" << i << ", [sp, #-16]!\n";
            declare_var(node->args[i].name);
        } else {
             std::cerr << "Error: Too many arguments (max 8 supported)" << std::endl;
             exit(1);
        }
    }
    
    node->body->accept(this);
    
    m_output << "    mov x0, #0\n";
    m_output << "    mov sp, x29\n"; 
    m_output << "    ldp x29, x30, [sp], #16\n";
    m_output << "    ret\n\n";
    
    pop_scope();
}

void Generator::visit(const ReturnStmt* node) {
    node->expr->accept(this);
    m_output << "    mov sp, x29\n";
    m_output << "    ldp x29, x30, [sp], #16\n";
    m_output << "    ret\n";
}

void Generator::visit(const ExprStmt* node) {
    node->expr->accept(this);
}

void Generator::visit(const VarDecl* node) {
    if (node->init) {
        node->init->accept(this);
        m_output << "    str x0, [sp, #-16]!\n";
    } else {
        if (node->array_size) {
            int bytes = *node->array_size * 16;
            m_output << "    sub sp, sp, #" << bytes << "\n";
        } else {
            m_output << "    sub sp, sp, #16\n";
        }
    }
    declare_var(node->name, node->array_size);
}

void Generator::visit(const AssignStmt* node) {
    auto var = find_var(node->name);
    if (!var.has_value()) {
        std::cerr << "Error: Undeclared variable: " << node->name << std::endl;
        exit(1);
    }
    node->value->accept(this);
    int offset = -(int)var->stack_offset;
    m_output << "    str x0, [x29, #" << offset << "]\n";
}

void Generator::visit(const ArrayAssignStmt* node) {
    auto var = find_var(node->name);
    if (!var.has_value()) {
        std::cerr << "Error: Undeclared variable: " << node->name << std::endl;
        exit(1);
    }
    node->value->accept(this);
    m_output << "    str x0, [sp, #-16]!\n";
    node->index->accept(this);
    m_output << "    mov x1, #16\n";
    m_output << "    mul x0, x0, x1\n";
    int base_offset = -(int)var->stack_offset;
    m_output << "    add x1, x29, #" << base_offset << "\n";
    m_output << "    add x1, x1, x0\n";
    m_output << "    ldr x0, [sp], #16\n";
    m_output << "    str x0, [x1]\n";
}

void Generator::visit(const PointerAssignStmt* node) {
    node->value->accept(this); // Result in x0
    m_output << "    str x0, [sp, #-16]!\n";
    node->ptr_expr->accept(this); // Address (value of p) in x0
    m_output << "    ldr x1, [sp], #16\n";
    m_output << "    str x1, [x0]\n";
}

void Generator::visit(const ScopeStmt* node) {
    size_t saved_stack_ptr = m_stack_ptr;
    push_scope();
    for (const auto& s : node->stmts) {
        s->accept(this);
    }
    size_t bytes_to_pop = m_stack_ptr - saved_stack_ptr;
    if (bytes_to_pop > 0) {
        m_output << "    add sp, sp, #" << bytes_to_pop << "\n";
    }
    m_stack_ptr = saved_stack_ptr;
    pop_scope();
}

void Generator::visit(const IfStmt* node) {
    std::string label_else = create_label();
    std::string label_end = create_label();
    
    node->condition->accept(this);
    m_output << "    cmp x0, #0\n";
    m_output << "    b.eq " << label_else << "\n"; 
    
    node->then_stmt->accept(this);
    m_output << "    b " << label_end << "\n";
    
    m_output << label_else << ":\n";
    if (node->else_stmt) {
        node->else_stmt->accept(this);
    }
    m_output << label_end << ":\n";
}

void Generator::visit(const WhileStmt* node) {
    std::string label_start = create_label();
    std::string label_end = create_label();
    
    m_output << label_start << ":\n";
    node->condition->accept(this);
    m_output << "    cmp x0, #0\n";
    m_output << "    b.eq " << label_end << "\n";
    
    node->body->accept(this);
    m_output << "    b " << label_start << "\n";
    m_output << label_end << ":\n";
}

void Generator::visit(const ForStmt* node) {
    size_t saved_stack_ptr = m_stack_ptr;
    push_scope();
    if (node->init) node->init->accept(this);
    
    std::string label_start = create_label();
    std::string label_end = create_label();
    
    m_output << label_start << ":\n";
    if (node->condition) {
        node->condition->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    b.eq " << label_end << "\n";
    }
    node->body->accept(this);
    if (node->increment) node->increment->accept(this);
    m_output << "    b " << label_start << "\n";
    m_output << label_end << ":\n";
    
    size_t bytes_to_pop = m_stack_ptr - saved_stack_ptr;
    if (bytes_to_pop > 0) {
        m_output << "    add sp, sp, #" << bytes_to_pop << "\n";
    }
    m_stack_ptr = saved_stack_ptr;
    pop_scope();
}

void Generator::visit(const IntLitExpr* node) {
    m_output << "    mov x0, #" << node->value << "\n";
}

void Generator::visit(const BoolLitExpr* node) {
    m_output << "    mov x0, #" << (node->value ? 1 : 0) << "\n";
}

void Generator::visit(const IdentifierExpr* node) {
    auto var = find_var(node->name);
    if (!var.has_value()) {
        std::cerr << "Error: Undeclared variable: " << node->name << std::endl;
        exit(1);
    }
    int offset = -(int)var->stack_offset;
    m_output << "    ldr x0, [x29, #" << offset << "]\n";
}

void Generator::visit(const ArrayAccessExpr* node) {
    auto var = find_var(node->name);
    if (!var.has_value()) {
        std::cerr << "Error: Undeclared variable: " << node->name << std::endl;
        exit(1);
    }
    node->index->accept(this);
    m_output << "    mov x1, #16\n";
    m_output << "    mul x0, x0, x1\n";
    int base_offset = -(int)var->stack_offset;
    m_output << "    add x1, x29, #" << base_offset << "\n";
    m_output << "    add x1, x1, x0\n";
    m_output << "    ldr x0, [x1]\n";
}

void Generator::visit(const CallExpr* node) {
    if (node->callee == "print") {
        node->args[0]->accept(this);
        m_output << "    mov x1, x0\n";
        m_output << "    adrp x0, fmt@PAGE\n";
        m_output << "    add x0, x0, fmt@PAGEOFF\n";
        m_output << "    bl _printf\n";
    } else {
        for (const auto& arg : node->args) {
            arg->accept(this);
            m_output << "    str x0, [sp, #-16]!\n";
        }
        for (int i = (int)node->args.size() - 1; i >= 0; --i) {
            m_output << "    ldr x" << i << ", [sp], #16\n";
        }
        m_output << "    bl _" << node->callee << "\n";
    }
}

void Generator::visit(const UnaryExpr* node) {
    if (node->op == TokenType::bang) {
        node->operand->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    cset x0, eq\n";
    } else if (node->op == TokenType::star) { // Dereference
        node->operand->accept(this); // Evaluates to an address
        m_output << "    ldr x0, [x0]\n";
    } else if (node->op == TokenType::amp) { // Address-of
        // We need the address of the operand, NOT its value.
        // This requires special handling because visit(IdentifierExpr) loads the value.
        // We can check the type of operand.
        if (const auto* ident = dynamic_cast<const IdentifierExpr*>(node->operand.get())) {
             auto var = find_var(ident->name);
             if (!var.has_value()) {
                 std::cerr << "Error: Undeclared variable: " << ident->name << std::endl;
                 exit(1);
             }
             int offset = -(int)var->stack_offset;
             m_output << "    add x0, x29, #" << offset << "\n";
        } else if (const auto* arr_access = dynamic_cast<const ArrayAccessExpr*>(node->operand.get())) {
             auto var = find_var(arr_access->name);
             arr_access->index->accept(this); // Index in x0
             m_output << "    mov x1, #16\n";
             m_output << "    mul x0, x0, x1\n";
             int base_offset = -(int)var->stack_offset;
             m_output << "    add x1, x29, #" << base_offset << "\n";
             m_output << "    add x0, x1, x0\n"; // Address in x0
        } else {
            std::cerr << "Error: Cannot take address of this expression." << std::endl;
            exit(1);
        }
    }
}

void Generator::visit(const BinaryExpr* node) {
    if (node->op == TokenType::amp_amp) {
        std::string label_false = create_label();
        std::string label_end = create_label();
        node->lhs->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    b.eq " << label_false << "\n";
        node->rhs->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    b.eq " << label_false << "\n";
        m_output << "    mov x0, #1\n";
        m_output << "    b " << label_end << "\n";
        m_output << label_false << ":\n";
        m_output << "    mov x0, #0\n";
        m_output << label_end << ":\n";
    } else if (node->op == TokenType::pipe_pipe) {
        std::string label_true = create_label();
        std::string label_end = create_label();
        node->lhs->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    b.ne " << label_true << "\n";
        node->rhs->accept(this);
        m_output << "    cmp x0, #0\n";
        m_output << "    b.ne " << label_true << "\n";
        m_output << "    mov x0, #0\n";
        m_output << "    b " << label_end << "\n";
        m_output << label_true << ":\n";
        m_output << "    mov x0, #1\n";
        m_output << label_end << ":\n";
    } else {
        node->rhs->accept(this);
        m_output << "    str x0, [sp, #-16]!\n";
        node->lhs->accept(this);
        m_output << "    ldr x1, [sp], #16\n";
        if (node->op == TokenType::plus) m_output << "    add x0, x0, x1\n";
        else if (node->op == TokenType::minus) m_output << "    sub x0, x0, x1\n";
        else if (node->op == TokenType::star) m_output << "    mul x0, x0, x1\n";
        else if (node->op == TokenType::slash) m_output << "    sdiv x0, x0, x1\n";
        else if (node->op == TokenType::eq_eq) { m_output << "    cmp x0, x1\n"; m_output << "    cset x0, eq\n"; }
        else if (node->op == TokenType::neq) { m_output << "    cmp x0, x1\n"; m_output << "    cset x0, ne\n"; }
        else if (node->op == TokenType::lt) { m_output << "    cmp x0, x1\n"; m_output << "    cset x0, lt\n"; }
        else if (node->op == TokenType::gt) { m_output << "    cmp x0, x1\n"; m_output << "    cset x0, gt\n"; }
    }
}
