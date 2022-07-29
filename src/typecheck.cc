#include <typecheck.hh>
#include <utils.hh>

void TypeChecker::check(AST *node) {
  for (auto child : *node->block.statements) {
    switch (child->type) {
      case ASTType::FunctionDef: check_function(child); break;
      default: {
        cerr << HERE << " UNHANDLED TYPE IN check: " << child->type
             << std::endl;
        exit(1);
      }
    }
  }
}

Variable *TypeChecker::find_var(std::string_view name) {
  for (int i = scopes.size() - 1; i >= 0; i--) {
    auto &vars = scopes[i].variables;
    if (auto var = vars.find(name); var != vars.end()) { return var->second; }
  }
  return nullptr;
}

void TypeChecker::push_var(Variable *var, Location loc) {
  auto &scope = scopes.back();
  if (scope.variables.count(var->name) > 0) {
    cerr << loc << ": Variable " << var->name << " already defined in scope"
         << std::endl;
    exit(1);
  }
  scope.variables[var->name] = var;
}

bool TypeChecker::check_valid_type(Type *type) {
  // TODO: Keep track of defined structs and look them up later.
  switch (type->base) {
    case BaseType::I32:
    case BaseType::Bool:
    case BaseType::Void: return true;
    default: {
      cerr << HERE << " UNHANDLED TYPE IN check_valid_type: " << type->base
           << std::endl;
      exit(1);
    }
  }
}

void TypeChecker::error(AST *node, const char *msg) {
  cerr << node->location << ": " << msg << endl;
  exit(1);
}

// Stubs
void TypeChecker::check_function(AST *node) {
  auto prev_func                 = curr_func;
  curr_func                      = node;
  functions[node->func_def.name] = node;
  push_scope();

  for (auto param : *node->func_def.params) {
    check_valid_type(param->type);
    push_var(param, node->location);
  }
  check_valid_type(node->func_def.return_type);

  check_block(node->func_def.body);

  pop_scope();
  curr_func = prev_func;
}

void TypeChecker::check_block(AST *node) {
  push_scope();
  for (auto child : *node->block.statements) check_statement(child);
  pop_scope();
}

void TypeChecker::check_statement(AST *node) {
  switch (node->type) {
    case ASTType::Block: check_block(node); return;
    case ASTType::Return: {
      if (!curr_func) { error(node, "Return statement outside of function"); }
      if (curr_func->func_def.return_type->base == BaseType::Void) {
        error(node, "Cannot return from void function");
      }
      auto ret_type = check_expression(node->unary.expr);
      if (*ret_type != *curr_func->func_def.return_type) {
        error(node, "Return type does not match function return type");
      }
      return;
    }
    case ASTType::VarDeclaration: {
      // TODO: Infer type?
      if (node->var_decl.init) {
        auto init_type = check_expression(node->var_decl.init);
        if (!node->var_decl.var->type) {
          node->var_decl.var->type = init_type;
        } else if (*node->var_decl.var->type != *init_type) {
          error(node, "Variable type does not match initializer type");
        }
      } else {
        if (!node->var_decl.var->type) {
          error(node, "Variable type cannot be inferred, specify explicitly");
        }
        check_valid_type(node->var_decl.var->type);
      }
      push_var(node->var_decl.var, node->location);
      return;
    }
    case ASTType::While: {
      auto cond_type = check_expression(node->while_loop.cond);
      if (cond_type->base != BaseType::Bool) {
        error(node, "Condition must be boolean");
      }
      check_statement(node->while_loop.body);
      return;
    }
    case ASTType::If: {
      auto cond_type = check_expression(node->if_stmt.cond);
      if (cond_type->base != BaseType::Bool) {
        error(node, "Condition must be boolean");
      }
      check_statement(node->if_stmt.body);
      if (node->if_stmt.els) { check_statement(node->if_stmt.els); }
      return;
    }
    default: check_expression(node); return;
  }
}

Type *TypeChecker::check_call(AST *node) {
  // TODO: Allow expressions evaluating to functions?
  if (node->call.callee->type != ASTType::Var) {
    error(node, "Functions need to explicitly be specified by name.");
  }
  auto &name = node->call.callee->var.name;

  // This is a hack, don't check the types of parameters
  if (name == "print" || name == "println") {
    for (auto arg : *node->call.args) check_expression(arg);
    return new Type(BaseType::Void);
  }

  if (functions.count(name) == 0) { error(node, "Function not found"); }
  auto func = functions[name];
  if (func->func_def.params->size() != node->call.args->size()) {
    error(node, "Number of arghuments does not match function signature");
  }
  auto &params = *func->func_def.params;
  for (int i = 0; i < params.size(); i++) {
    auto param    = params[i];
    auto arg      = node->call.args->at(i);
    auto arg_type = check_expression(arg);
    if (*param->type != *arg_type) {
      error(node, "Argument type does not match function parameter type");
    }
  }

  return func->func_def.return_type;
}

Type *TypeChecker::check_expression(AST *node) {
  switch (node->type) {
    case ASTType::Call: return check_call(node);
    case ASTType::IntLiteral: return new Type(BaseType::I32);
    case ASTType::BoolLiteral: return new Type(BaseType::Bool);
    // THIS IS AN UGLY HACK, FIX STRINGS PLS
    case ASTType::StringLiteral: return new Type(BaseType::Void);
    case ASTType::Var: {
      auto var = find_var(node->var.name);
      if (var == nullptr) { error(node, "Variable not found"); }
      return var->type;
    }
    // TODO: Allow more comlex binary expressions, will eventually need support
    // for pointers
    case ASTType::Plus:
    case ASTType::Minus:
    case ASTType::Multiply:
    case ASTType::Divide: {
      auto lhs_type = check_expression(node->binary.lhs);
      auto rhs_type = check_expression(node->binary.rhs);
      if (lhs_type->base != BaseType::I32 || rhs_type->base != BaseType::I32) {
        error(node, "Operands must be integers");
      }
      return new Type(BaseType::I32);
    }

    case ASTType::LessThan:
    case ASTType::GreaterThan: {
      auto lhs_type = check_expression(node->binary.lhs);
      auto rhs_type = check_expression(node->binary.rhs);
      if (lhs_type->base != BaseType::I32 || rhs_type->base != BaseType::I32) {
        error(node, "Operands must be integers");
      }
      return new Type(BaseType::Bool);
    }

    case ASTType::Assignment: {
      auto lhs = check_expression(node->binary.lhs);
      auto rhs = check_expression(node->binary.rhs);
      if (*lhs != *rhs) {
        error(node, "Variable type does not match assignment type");
      }
      return lhs;
    }

    default: break;
  }
  cerr << HERE << " UNHANDLED TYPE IN check_expression: " << node->type
       << std::endl;
  exit(1);
}