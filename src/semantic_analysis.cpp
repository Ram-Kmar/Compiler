#include "semantic_analysis.h"
#include <iostream>
#include <vector>

SemanticAnalyzer::SemanticAnalyzer(Program* program) : m_prog(program) {
    // Register built-in functions
    m_functions["print"] = {Type::Void(), {Type::Int()}};
}

void SemanticAnalyzer::report_error(const std::string& message, const Node* node) {
    std::cerr << "Semantic Error: " << message << " at " << node->line << ":" << node->col << std::endl;
    exit(1);
}

void SemanticAnalyzer::push_scope() {
    m_scopes.push_back({});
}

void SemanticAnalyzer::pop_scope() {
    m_scopes.pop_back();
}

void SemanticAnalyzer::declare_var(const std::string& name, Type type, const Node* node, std::optional<int> array_size) {
    if (m_scopes.back().count(name)) {
        report_error("Variable '" + name + "' already declared in this scope.", node);
    }
    m_scopes.back()[name] = {type, array_size};
}

std::optional<Symbol> SemanticAnalyzer::find_var(const std::string& name) {
    for (auto it = m_scopes.rbegin(); it != m_scopes.rend(); ++it) {
        if (it->count(name)) {
            return it->at(name);
        }
    }
    return std::nullopt;
}

void SemanticAnalyzer::analyze() {
    push_scope(); // Global scope

    // 1. Register all functions first
    for (const auto& func : m_prog->functions) {
        register_function(func.get());
    }

    // 2. Analyze globals
    for (const auto& stmt : m_prog->globals) {
        analyze_stmt(stmt.get());
    }

    // 3. Analyze function bodies
    for (const auto& func : m_prog->functions) {
        analyze_function(func.get());
    }

    pop_scope();
}

void SemanticAnalyzer::register_function(const Function* func) {
    if (m_functions.count(func->name)) {
        report_error("Function '" + func->name + "' already defined.", func);
    }
    std::vector<Type> arg_types;
    for (const auto& arg : func->args) {
        arg_types.push_back(arg.type);
    }
    m_functions[func->name] = {func->return_type, arg_types};
}

void SemanticAnalyzer::analyze_function(const Function* func) {
    m_current_func_return_type = func->return_type;
    push_scope();

    for (const auto& arg : func->args) {
        declare_var(arg.name, arg.type, func);
    }

    analyze_stmt(func->body.get());

    pop_scope();
    m_current_func_return_type = std::nullopt;
}

void SemanticAnalyzer::analyze_stmt(const Stmt* stmt) {
    if (const auto* ret_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Type expr_type = analyze_expr(ret_stmt->expr.get());
        if (!m_current_func_return_type.has_value()) {
            // Implicit main returns int, usually
            if (expr_type != Type::Int()) {
                report_error("Global return statements must return int.", ret_stmt);
            }
        } else {
            if (expr_type != m_current_func_return_type.value()) {
                 // Simple mismatch check
                report_error("Return type mismatch.", ret_stmt);
            }
        }
    } 
    else if (const auto* expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        analyze_expr(expr_stmt->expr.get());
    }
    else if (const auto* var_decl = dynamic_cast<const VarDecl*>(stmt)) {
        if (var_decl->init) {
            Type init_type = analyze_expr(var_decl->init.get());
            if (init_type != var_decl->type) {
                report_error("Type mismatch in initialization of '" + var_decl->name + "'.", var_decl);
            }
        }
        declare_var(var_decl->name, var_decl->type, var_decl, var_decl->array_size);
    }
    else if (const auto* assign_stmt = dynamic_cast<const AssignStmt*>(stmt)) {
        auto var = find_var(assign_stmt->name);
        if (!var.has_value()) {
            report_error("Undeclared variable '" + assign_stmt->name + "'.", assign_stmt);
        }
        if (var->array_size.has_value()) {
            report_error("Cannot assign directly to array '" + assign_stmt->name + "'. Use indexing.", assign_stmt);
        }
        Type expr_type = analyze_expr(assign_stmt->value.get());
        if (expr_type != var->type) {
            report_error("Type mismatch in assignment to '" + assign_stmt->name + "'.", assign_stmt);
        }
    }
    else if (const auto* arr_assign = dynamic_cast<const ArrayAssignStmt*>(stmt)) {
        auto var = find_var(arr_assign->name);
        if (!var.has_value()) {
            report_error("Undeclared variable '" + arr_assign->name + "'.", arr_assign);
        }
        if (!var->array_size.has_value()) {
            report_error("Variable '" + arr_assign->name + "' is not an array.", arr_assign);
        }
        Type idx_type = analyze_expr(arr_assign->index.get());
        if (idx_type != Type::Int()) {
            report_error("Array index must be int.", arr_assign);
        }
        Type val_type = analyze_expr(arr_assign->value.get());
        if (val_type != var->type) {
            report_error("Type mismatch in array assignment to '" + arr_assign->name + "'.", arr_assign);
        }
    }
    else if (const auto* ptr_assign = dynamic_cast<const PointerAssignStmt*>(stmt)) {
        Type ptr_type = analyze_expr(ptr_assign->ptr_expr.get());
        // ptr_expr is 'p' in '*p = ...'. So ptr_expr must be a pointer type.
        if (ptr_type.ptr_level == 0) {
            report_error("Cannot dereference non-pointer type in assignment.", ptr_assign);
        }
        Type val_type = analyze_expr(ptr_assign->value.get());
        Type target_type = ptr_type;
        target_type.ptr_level--;
        
        if (val_type != target_type) {
            report_error("Type mismatch in pointer assignment.", ptr_assign);
        }
    }
    else if (const auto* scope_stmt = dynamic_cast<const ScopeStmt*>(stmt)) {
        push_scope();
        for (const auto& s : scope_stmt->stmts) {
            analyze_stmt(s.get());
        }
        pop_scope();
    }
    else if (const auto* if_stmt = dynamic_cast<const IfStmt*>(stmt)) {
        Type cond_type = analyze_expr(if_stmt->condition.get());
        if (cond_type != Type::Bool()) {
            report_error("If condition must be bool.", if_stmt);
        }
        analyze_stmt(if_stmt->then_stmt.get());
        if (if_stmt->else_stmt) {
            analyze_stmt(if_stmt->else_stmt.get());
        }
    }
    else if (const auto* while_stmt = dynamic_cast<const WhileStmt*>(stmt)) {
        Type cond_type = analyze_expr(while_stmt->condition.get());
        if (cond_type != Type::Bool()) {
            report_error("While condition must be bool.", while_stmt);
        }
        analyze_stmt(while_stmt->body.get());
    }
    else if (const auto* for_stmt = dynamic_cast<const ForStmt*>(stmt)) {
        push_scope(); 
        if (for_stmt->init) analyze_stmt(for_stmt->init.get());
        if (for_stmt->condition) {
            Type cond_type = analyze_expr(for_stmt->condition.get());
            if (cond_type != Type::Bool()) {
                report_error("For condition must be bool.", for_stmt);
            }
        }
        if (for_stmt->increment) analyze_stmt(for_stmt->increment.get());
        analyze_stmt(for_stmt->body.get());
        pop_scope();
    }
}

Type SemanticAnalyzer::analyze_expr(const Expr* expr) {
    if (const auto* int_lit = dynamic_cast<const IntLitExpr*>(expr)) {
        return Type::Int();
    } 
    else if (const auto* bool_lit = dynamic_cast<const BoolLitExpr*>(expr)) {
        return Type::Bool();
    }
    else if (const auto* ident_expr = dynamic_cast<const IdentifierExpr*>(expr)) {
        auto var = find_var(ident_expr->name);
        if (!var.has_value()) {
            report_error("Undeclared variable '" + ident_expr->name + "'.", ident_expr);
        }
        if (var->array_size.has_value()) {
            report_error("Variable '" + ident_expr->name + "' is an array, must be indexed.", ident_expr);
        }
        return var->type;
    }
    else if (const auto* arr_access = dynamic_cast<const ArrayAccessExpr*>(expr)) {
        auto var = find_var(arr_access->name);
        if (!var.has_value()) {
            report_error("Undeclared variable '" + arr_access->name + "'.", arr_access);
        }
        if (!var->array_size.has_value()) {
            report_error("Variable '" + arr_access->name + "' is not an array.", arr_access);
        }
        Type idx_type = analyze_expr(arr_access->index.get());
        if (idx_type != Type::Int()) {
            report_error("Array index must be int.", arr_access);
        }
        return var->type;
    }
    else if (const auto* call_expr = dynamic_cast<const CallExpr*>(expr)) {
        auto it = m_functions.find(call_expr->callee);
        if (it == m_functions.end()) {
            report_error("Undefined function '" + call_expr->callee + "'.", call_expr);
        }
        
        const auto& signature = it->second;
        if (call_expr->args.size() != signature.arg_types.size()) {
            report_error("Argument count mismatch.", call_expr);
        }
        
        for (size_t i = 0; i < call_expr->args.size(); ++i) {
            Type arg_type = analyze_expr(call_expr->args[i].get());
            if (arg_type != signature.arg_types[i]) {
                report_error("Argument type mismatch.", call_expr->args[i].get());
            }
        }
        return signature.return_type;
    }
    else if (const auto* bin_expr = dynamic_cast<const BinaryExpr*>(expr)) {
        Type lhs_type = analyze_expr(bin_expr->lhs.get());
        Type rhs_type = analyze_expr(bin_expr->rhs.get());
        
        if (bin_expr->op == TokenType::plus || bin_expr->op == TokenType::minus ||
            bin_expr->op == TokenType::star || bin_expr->op == TokenType::slash) {
            if (lhs_type != Type::Int() || rhs_type != Type::Int()) {
                report_error("Math operands must be int.", bin_expr);
            }
            return Type::Int();
        }
        else if (bin_expr->op == TokenType::amp_amp || bin_expr->op == TokenType::pipe_pipe) {
            if (lhs_type != Type::Bool() || rhs_type != Type::Bool()) {
                report_error("Logic operands must be bool.", bin_expr);
            }
            return Type::Bool();
        }
        else {
             if (bin_expr->op == TokenType::eq_eq || bin_expr->op == TokenType::neq) {
                 if (lhs_type != rhs_type) {
                     report_error("Comparison operands must be same type.", bin_expr);
                 }
                 return Type::Bool();
             }
             else {
                 if (lhs_type != Type::Int() || rhs_type != Type::Int()) {
                     report_error("Ordered comparison operands must be int.", bin_expr);
                 }
                 return Type::Bool();
             }
        }
    }
    else if (const auto* unary_expr = dynamic_cast<const UnaryExpr*>(expr)) {
        Type operand_type = analyze_expr(unary_expr->operand.get());
        if (unary_expr->op == TokenType::bang) {
            if (operand_type != Type::Bool()) {
                report_error("! operand must be bool.", unary_expr);
            }
            return Type::Bool();
        }
        else if (unary_expr->op == TokenType::star) { // Dereference
            if (operand_type.ptr_level == 0) {
                report_error("Cannot dereference a non-pointer type.", unary_expr);
            }
            Type new_type = operand_type;
            new_type.ptr_level--;
            return new_type;
        }
        else if (unary_expr->op == TokenType::amp) { // Address-of
            // TODO: strict l-value check? For now, we assume operand is valid for & 
            // but normally &5 is invalid.
            // Check if operand is an IdentifierExpr or ArrayAccessExpr
            if (dynamic_cast<const IdentifierExpr*>(unary_expr->operand.get()) ||
                dynamic_cast<const ArrayAccessExpr*>(unary_expr->operand.get())) {
                Type new_type = operand_type;
                new_type.ptr_level++;
                return new_type;
            } else {
                 report_error("Cannot take address of r-value.", unary_expr);
            }
        }
    }
    
    report_error("Unknown expression type.", expr);
    return Type::Void();
}
