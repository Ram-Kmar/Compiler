#include "semantic_analysis.h"
#include <iostream>
#include <vector>

SemanticAnalyzer::SemanticAnalyzer(Program* program) : m_prog(program) {
    // Register built-in functions
    // print(int) -> void (or int, let's say void for now, but usually used as stmt)
    m_functions["print"] = {Type::Void, {Type::Int}};
}

void SemanticAnalyzer::push_scope() {
    m_scopes.push_back({});
}

void SemanticAnalyzer::pop_scope() {
    m_scopes.pop_back();
}

void SemanticAnalyzer::declare_var(const std::string& name, Type type) {
    if (m_scopes.back().count(name)) {
        std::cerr << "Semantic Error: Variable '" << name << "' already declared in this scope." << std::endl;
        exit(1);
    }
    m_scopes.back()[name] = {type};
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

    // 1. Register all functions first (to allow forward references or recursion)
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
        std::cerr << "Semantic Error: Function '" << func->name << "' already defined." << std::endl;
        exit(1);
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

    // Declare arguments in scope
    for (const auto& arg : func->args) {
        declare_var(arg.name, arg.type);
    }

    // Analyze body
    // The body is a ScopeStmt, so it will push another scope.
    // However, arguments should be accessible.
    // Parser creates a ScopeStmt for the body. 
    // If we visit the ScopeStmt, it pushes a NEW scope.
    // That means arguments are in the scope SURROUNDING the body block.
    // That works fine.
    
    // We need to unwrap the ScopeStmt or just visit it. 
    // If we visit it, it adds a layer. 
    // Function: [Args Scope] -> [Body Scope] -> Stmts. 
    // This is correct C-like scoping.
    analyze_stmt(func->body.get());

    pop_scope();
    m_current_func_return_type = std::nullopt;
}

void SemanticAnalyzer::analyze_stmt(const Stmt* stmt) {
    if (const auto* ret_stmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        Type expr_type = analyze_expr(ret_stmt->expr.get());
        if (!m_current_func_return_type.has_value()) {
            // Allow global return, effectively inside implicit 'main' returning int
            if (expr_type != Type::Int) {
                std::cerr << "Semantic Error: Global return statements must return int." << std::endl;
                exit(1);
            }
        } else {
            if (expr_type != m_current_func_return_type.value()) {
                std::cerr << "Semantic Error: Return type mismatch. Expected " 
                          << (m_current_func_return_type.value() == Type::Int ? "int" : "void")
                          << ", got " << (expr_type == Type::Int ? "int" : "void") << std::endl;
                exit(1);
            }
        }
    } 
    else if (const auto* expr_stmt = dynamic_cast<const ExprStmt*>(stmt)) {
        analyze_expr(expr_stmt->expr.get());
    }
    else if (const auto* var_decl = dynamic_cast<const VarDecl*>(stmt)) {
        Type init_type = analyze_expr(var_decl->init.get());
        if (init_type != var_decl->type) {
            std::cerr << "Semantic Error: Type mismatch in initialization of '" << var_decl->name << "'." << std::endl;
            exit(1);
        }
        declare_var(var_decl->name, var_decl->type);
    }
    else if (const auto* assign_stmt = dynamic_cast<const AssignStmt*>(stmt)) {
        auto var = find_var(assign_stmt->name);
        if (!var.has_value()) {
            std::cerr << "Semantic Error: Undeclared variable '" << assign_stmt->name << "'." << std::endl;
            exit(1);
        }
        Type expr_type = analyze_expr(assign_stmt->value.get());
        if (expr_type != var->type) {
            std::cerr << "Semantic Error: Type mismatch in assignment to '" << assign_stmt->name << "'." << std::endl;
            exit(1);
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
        if (cond_type != Type::Bool) {
            std::cerr << "Semantic Error: If condition must be bool." << std::endl;
            exit(1);
        }
        analyze_stmt(if_stmt->then_stmt.get());
        if (if_stmt->else_stmt) {
            analyze_stmt(if_stmt->else_stmt.get());
        }
    }
    else if (const auto* while_stmt = dynamic_cast<const WhileStmt*>(stmt)) {
        Type cond_type = analyze_expr(while_stmt->condition.get());
        if (cond_type != Type::Bool) {
            std::cerr << "Semantic Error: While condition must be bool." << std::endl;
            exit(1);
        }
        analyze_stmt(while_stmt->body.get());
    }
}

Type SemanticAnalyzer::analyze_expr(const Expr* expr) {
    if (const auto* int_lit = dynamic_cast<const IntLitExpr*>(expr)) {
        return Type::Int;
    } 
    else if (const auto* bool_lit = dynamic_cast<const BoolLitExpr*>(expr)) {
        return Type::Bool;
    }
    else if (const auto* ident_expr = dynamic_cast<const IdentifierExpr*>(expr)) {
        auto var = find_var(ident_expr->name);
        if (!var.has_value()) {
            std::cerr << "Semantic Error: Undeclared variable '" << ident_expr->name << "'." << std::endl;
            exit(1);
        }
        return var->type;
    }
    else if (const auto* call_expr = dynamic_cast<const CallExpr*>(expr)) {
        auto it = m_functions.find(call_expr->callee);
        if (it == m_functions.end()) {
            std::cerr << "Semantic Error: Undefined function '" << call_expr->callee << "'." << std::endl;
            exit(1);
        }
        
        const auto& signature = it->second;
        if (call_expr->args.size() != signature.arg_types.size()) {
            std::cerr << "Semantic Error: Argument count mismatch for '" << call_expr->callee 
                      << "'. Expected " << signature.arg_types.size() 
                      << ", got " << call_expr->args.size() << "." << std::endl;
            exit(1);
        }
        
        for (size_t i = 0; i < call_expr->args.size(); ++i) {
            Type arg_type = analyze_expr(call_expr->args[i].get());
            if (arg_type != signature.arg_types[i]) {
                std::cerr << "Semantic Error: Argument " << (i+1) << " type mismatch for '" << call_expr->callee << "'." << std::endl;
                exit(1);
            }
        }
        return signature.return_type;
    }
    else if (const auto* bin_expr = dynamic_cast<const BinaryExpr*>(expr)) {
        Type lhs_type = analyze_expr(bin_expr->lhs.get());
        Type rhs_type = analyze_expr(bin_expr->rhs.get());
        
        // Math
        if (bin_expr->op == TokenType::plus || bin_expr->op == TokenType::minus ||
            bin_expr->op == TokenType::star || bin_expr->op == TokenType::slash) {
            if (lhs_type != Type::Int || rhs_type != Type::Int) {
                std::cerr << "Semantic Error: Math operands must be int." << std::endl;
                exit(1);
            }
            return Type::Int;
        }
        // Logic
        else if (bin_expr->op == TokenType::amp_amp || bin_expr->op == TokenType::pipe_pipe) {
            if (lhs_type != Type::Bool || rhs_type != Type::Bool) {
                std::cerr << "Semantic Error: Logic operands must be bool." << std::endl;
                exit(1);
            }
            return Type::Bool;
        }
        // Comparison
        else {
             // == != support both (types must match)
             if (bin_expr->op == TokenType::eq_eq || bin_expr->op == TokenType::neq) {
                 if (lhs_type != rhs_type) {
                     std::cerr << "Semantic Error: Comparison operands must be same type." << std::endl;
                     exit(1);
                 }
                 return Type::Bool;
             }
             // < > only Int
             else {
                 if (lhs_type != Type::Int || rhs_type != Type::Int) {
                     std::cerr << "Semantic Error: Ordered comparison operands must be int." << std::endl;
                     exit(1);
                 }
                 return Type::Bool;
             }
        }
    }
    else if (const auto* unary_expr = dynamic_cast<const UnaryExpr*>(expr)) {
        Type operand_type = analyze_expr(unary_expr->operand.get());
        if (unary_expr->op == TokenType::bang) {
            if (operand_type != Type::Bool) {
                std::cerr << "Semantic Error: ! operand must be bool." << std::endl;
                exit(1);
            }
            return Type::Bool;
        }
    }
    
    std::cerr << "Semantic Error: Unknown expression type." << std::endl;
    exit(1);
}
