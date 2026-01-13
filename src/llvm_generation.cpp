#include "llvm_generation.h"
#include <iostream>

LLVMGenerator::LLVMGenerator(const Program* root) : m_root(root) {
}

std::string LLVMGenerator::new_reg() {
    return "%r" + std::to_string(m_reg_count++);
}

std::string LLVMGenerator::new_label() {
    return "L" + std::to_string(m_label_count++);
}

void LLVMGenerator::push_scope() {
    m_scopes.push_back({});
}

void LLVMGenerator::pop_scope() {
    m_scopes.pop_back();
}

void LLVMGenerator::declare_var(const std::string& name, Type type) {
    m_scopes.back()[name] = { "%" + name + ".addr", type };
}

std::optional<LLVMVarInfo> LLVMGenerator::find_var(const std::string& name) {
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        if (it->count(name)) {
            return it->at(name);
        }
    }
    return std::nullopt;
}

std::string LLVMGenerator::to_llvm_type(Type type) {
    std::string base;
    switch (type.base) {
        case Type::Base::Int: base = "i32"; break;
        case Type::Base::Bool: base = "i1"; break;
        case Type::Base::Void: base = "void"; break;
    }
    for (int i = 0; i < type.ptr_level; ++i) {
        base += "*";
    }
    return base;
}

std::string LLVMGenerator::generate() {
    m_root->accept(this);
    return m_output.str();
}

void LLVMGenerator::visit(const Program* node) {
    m_output << "declare i32 @printf(i8*, ...)\n";
    m_output << "@.str = private unnamed_addr constant [4 x i8] [i8 37, i8 100, i8 10, i8 0]\n\n";

    bool has_main = false;
    for (const auto& func : node->functions) {
        if (func->name == "main") has_main = true;
        func->accept(this);
    }

    if (!has_main && !node->globals.empty()) {
        m_output << "define i32 @main() {\n";
        m_output << "entry:\n";
        m_reg_count = 0;
        push_scope();
        for (const auto& stmt : node->globals) {
            stmt->accept(this);
        }
        m_output << "  ret i32 0\n";
        m_output << "}\n\n";
        pop_scope();
    }
}

void LLVMGenerator::visit(const Function* node) {
    m_reg_count = 0;
    m_output << "define " << to_llvm_type(node->return_type) << " @" << node->name << "(";
    for (size_t i = 0; i < node->args.size(); ++i) {
        m_output << to_llvm_type(node->args[i].type) << " %" << node->args[i].name;
        if (i < node->args.size() - 1) m_output << ", ";
    }
    m_output << ") {\n";
    m_output << "entry:\n";
    
    push_scope();
    
    // Alloca and store arguments
    for (const auto& arg : node->args) {
        std::string addr = "%" + arg.name + ".addr";
        m_output << "  " << addr << " = alloca " << to_llvm_type(arg.type) << "\n";
        m_output << "  store " << to_llvm_type(arg.type) << " %" << arg.name << ", " << to_llvm_type(arg.type) << "* " << addr << "\n";
        declare_var(arg.name, arg.type);
    }
    
    node->body->accept(this);
    
    if (node->return_type.base == Type::Base::Void) {
        m_output << "  ret void\n";
    } else {
        // Find if last instruction was a return
        m_output << "  ret " << to_llvm_type(node->return_type) << " 0\n";
    }
    
    m_output << "}\n\n";
    pop_scope();
}

void LLVMGenerator::visit(const IntLitExpr* node) {
    m_last_reg = std::to_string(node->value);
}

void LLVMGenerator::visit(const BoolLitExpr* node) {
    m_last_reg = node->value ? "1" : "0";
}

void LLVMGenerator::visit(const IdentifierExpr* node) {
    auto var = find_var(node->name);
    m_last_reg = new_reg();
    m_output << "  " << m_last_reg << " = load " << to_llvm_type(var->type) << ", " << to_llvm_type(var->type) << "* " << var->name << "\n";
}

void LLVMGenerator::visit(const ArrayAccessExpr* node) {
    auto var = find_var(node->name);
    node->index->accept(this);
    std::string index_reg = m_last_reg;
    
    std::string ptr_reg = new_reg();
    Type elem_type = var->type; 
    m_output << "  " << ptr_reg << " = getelementptr inbounds " << to_llvm_type(elem_type) << ", " << to_llvm_type(elem_type) << "* " << var->name << ", i32 " << index_reg << "\n";
    
    m_last_reg = new_reg();
    m_output << "  " << m_last_reg << " = load " << to_llvm_type(elem_type) << ", " << to_llvm_type(elem_type) << "* " << ptr_reg << "\n";
}

void LLVMGenerator::visit(const CallExpr* node) {
    if (node->callee == "print") {
        node->args[0]->accept(this);
        std::string val_reg = m_last_reg;
        std::string call_reg = new_reg();
        m_output << "  " << call_reg << " = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 " << val_reg << ")\n";
        m_last_reg = call_reg;
    } else {
        std::vector<std::string> arg_regs;
        for (const auto& arg : node->args) {
            arg->accept(this);
            arg_regs.push_back(m_last_reg);
        }
        m_last_reg = new_reg();
        m_output << "  " << m_last_reg << " = call i32 @" << node->callee << "(";
        for (size_t i = 0; i < arg_regs.size(); ++i) {
            m_output << "i32 " << arg_regs[i]; 
            if (i < arg_regs.size() - 1) m_output << ", ";
        }
        m_output << ")\n";
    }
}

void LLVMGenerator::visit(const UnaryExpr* node) {
    node->operand->accept(this);
    std::string op_reg = m_last_reg;
    m_last_reg = new_reg();
    if (node->op == TokenType::bang) {
        m_output << "  " << m_last_reg << " = xor i1 " << op_reg << ", 1\n";
    } else if (node->op == TokenType::star) {
        m_output << "  " << m_last_reg << " = load i32, i32* " << op_reg << "\n";
    } else if (node->op == TokenType::amp) {
        if (const auto* ident = dynamic_cast<const IdentifierExpr*>(node->operand.get())) {
            auto var = find_var(ident->name);
            m_last_reg = var->name;
        }
    }
}

void LLVMGenerator::visit(const BinaryExpr* node) {
    if (node->op == TokenType::amp_amp) {
        std::string label_check_rhs = new_label();
        std::string label_end = new_label();
        
        node->lhs->accept(this);
        std::string lhs_reg = m_last_reg;
        
        // Ensure lhs is i1. If it looks like a register (starts with %), assumes it is i1 or we need to check type.
        // For simplicity, assuming i1 if coming from bool context. 
        // Realistically we should check the expression type or cast. 
        // But simpler: just compare ne 0 if we suspect i32? 
        // Let's rely on the fact that logical ops usually take boolean/comparison results (i1).
        
        std::string res_addr = "%and_res" + std::to_string(m_reg_count++); // Ensure unique name
        m_output << "  " << res_addr << " = alloca i1\n";
        m_output << "  store i1 0, i1* " << res_addr << "\n"; // Default false
        
        m_output << "  br i1 " << lhs_reg << ", label %" << label_check_rhs << ", label %" << label_end << "\n";
        
        m_output << label_check_rhs << ":\n";
        node->rhs->accept(this);
        m_output << "  store i1 " << m_last_reg << ", i1* " << res_addr << "\n";
        m_output << "  br label %" << label_end << "\n";
        
        m_output << label_end << ":\n";
        m_last_reg = new_reg();
        m_output << "  " << m_last_reg << " = load i1, i1* " << res_addr << "\n";

    } else if (node->op == TokenType::pipe_pipe) {
        std::string label_check_rhs = new_label();
        std::string label_end = new_label();
        
        node->lhs->accept(this);
        std::string lhs_reg = m_last_reg;
        
        std::string res_addr = "%or_res" + std::to_string(m_reg_count++);
        m_output << "  " << res_addr << " = alloca i1\n";
        m_output << "  store i1 1, i1* " << res_addr << "\n"; // Default true
        
        m_output << "  br i1 " << lhs_reg << ", label %" << label_end << ", label %" << label_check_rhs << "\n";
        
        m_output << label_check_rhs << ":\n";
        node->rhs->accept(this);
        m_output << "  store i1 " << m_last_reg << ", i1* " << res_addr << "\n";
        m_output << "  br label %" << label_end << "\n";
        
        m_output << label_end << ":\n";
        m_last_reg = new_reg();
        m_output << "  " << m_last_reg << " = load i1, i1* " << res_addr << "\n";

    } else {
        node->lhs->accept(this);
        std::string lhs_reg = m_last_reg;
        node->rhs->accept(this);
        std::string rhs_reg = m_last_reg;
        
        m_last_reg = new_reg();
        if (node->op == TokenType::plus) m_output << "  " << m_last_reg << " = add i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::minus) m_output << "  " << m_last_reg << " = sub i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::star) m_output << "  " << m_last_reg << " = mul i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::slash) m_output << "  " << m_last_reg << " = sdiv i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::eq_eq) m_output << "  " << m_last_reg << " = icmp eq i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::neq) m_output << "  " << m_last_reg << " = icmp ne i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::lt) m_output << "  " << m_last_reg << " = icmp slt i32 " << lhs_reg << ", " << rhs_reg << "\n";
        else if (node->op == TokenType::gt) m_output << "  " << m_last_reg << " = icmp sgt i32 " << lhs_reg << ", " << rhs_reg << "\n";
    }
}

void LLVMGenerator::visit(const ReturnStmt* node) {
    node->expr->accept(this);
    m_output << "  ret i32 " << m_last_reg << "\n";
}

void LLVMGenerator::visit(const ExprStmt* node) {
    node->expr->accept(this);
}

void LLVMGenerator::visit(const VarDecl* node) {
    std::string addr = "%" + node->name + ".addr";
    m_output << "  " << addr << " = alloca " << to_llvm_type(node->type) << "\n";
    declare_var(node->name, node->type);
    if (node->init) {
        node->init->accept(this);
        m_output << "  store " << to_llvm_type(node->type) << " " << m_last_reg << ", " << to_llvm_type(node->type) << "* " << addr << "\n";
    }
}

void LLVMGenerator::visit(const AssignStmt* node) {
    auto var = find_var(node->name);
    node->value->accept(this);
    m_output << "  store " << to_llvm_type(var->type) << " " << m_last_reg << ", " << to_llvm_type(var->type) << "* " << var->name << "\n";
}

void LLVMGenerator::visit(const ArrayAssignStmt* node) {
    auto var = find_var(node->name);
    node->index->accept(this);
    std::string index_reg = m_last_reg;
    node->value->accept(this);
    std::string val_reg = m_last_reg;
    
    std::string ptr_reg = new_reg();
    m_output << "  " << ptr_reg << " = getelementptr inbounds " << to_llvm_type(var->type) << ", " << to_llvm_type(var->type) << "* " << var->name << ", i32 " << index_reg << "\n";
    m_output << "  store i32 " << val_reg << ", i32* " << ptr_reg << "\n";
}

void LLVMGenerator::visit(const PointerAssignStmt* node) {
    node->ptr_expr->accept(this);
    std::string ptr_reg = m_last_reg;
    node->value->accept(this);
    std::string val_reg = m_last_reg;
    m_output << "  store i32 " << val_reg << ", i32* " << ptr_reg << "\n";
}

void LLVMGenerator::visit(const ScopeStmt* node) {
    push_scope();
    for (const auto& stmt : node->stmts) {
        stmt->accept(this);
    }
    pop_scope();
}

void LLVMGenerator::visit(const IfStmt* node) {
    std::string label_then = new_label();
    std::string label_else = new_label();
    std::string label_end = new_label();
    
    node->condition->accept(this);
    m_output << "  br i1 " << m_last_reg << ", label %" << label_then << ", label %" << label_else << "\n";
    
    m_output << label_then << ":\n";
    node->then_stmt->accept(this);
    m_output << "  br label %" << label_end << "\n";
    
    m_output << label_else << ":\n";
    if (node->else_stmt) {
        node->else_stmt->accept(this);
    }
    m_output << "  br label %" << label_end << "\n";
    
    m_output << label_end << ":\n";
}

void LLVMGenerator::visit(const WhileStmt* node) {
    std::string label_cond = new_label();
    std::string label_body = new_label();
    std::string label_end = new_label();
    
    m_output << "  br label %" << label_cond << "\n";
    m_output << label_cond << ":\n";
    node->condition->accept(this);
    m_output << "  br i1 " << m_last_reg << ", label %" << label_body << ", label %" << label_end << "\n";
    
    m_output << label_body << ":\n";
    node->body->accept(this);
    m_output << "  br label %" << label_cond << "\n";
    
    m_output << label_end << ":\n";
}

void LLVMGenerator::visit(const ForStmt* node) {
    push_scope();
    if (node->init) node->init->accept(this);
    
    std::string label_cond = new_label();
    std::string label_body = new_label();
    std::string label_inc = new_label();
    std::string label_end = new_label();
    
    m_output << "  br label %" << label_cond << "\n";
    m_output << label_cond << ":\n";
    if (node->condition) {
        node->condition->accept(this);
        m_output << "  br i1 " << m_last_reg << ", label %" << label_body << ", label %" << label_end << "\n";
    } else {
        m_output << "  br label %" << label_body << "\n";
    }
    
    m_output << label_body << ":\n";
    node->body->accept(this);
    m_output << "  br label %" << label_inc << "\n";
    
    m_output << label_inc << ":\n";
    if (node->increment) node->increment->accept(this);
    m_output << "  br label %" << label_cond << "\n";
    
    m_output << label_end << ":\n";
    pop_scope();
}