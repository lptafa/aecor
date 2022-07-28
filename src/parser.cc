#include "parser.hh"

#include <iostream>

#include "tokens.hh"

#define UNHANDLED_TYPE()                                                       \
  cerr << token().location << ": Unexpected token: " << token().type << endl;  \
  cerr << HERE << " Source location: " << __FUNCTION__ << endl;                \
  exit(1);

Token &Parser::consume_impl(TokenType token_type, const char *sloc) {
  auto &ret = expect_impl(token_type, sloc);
  curr++;
  return ret;
};

Token &Parser::expect_impl(TokenType token_type, const char *sloc) {
  if (token_is(token_type)) return token();
  std::cerr << token().location << ": expected " << token_type << " but got "
            << token().type << std::endl;
  std::cerr << sloc << " Location in compiler" << std::endl;
  exit(1);
};

bool Parser::consume_if(TokenType token_type) {
  if (token_is(token_type)) {
    curr++;
    return true;
  }
  return false;
}

void Parser::consume_line_end() {
  if (token().type == TokenType::Semicolon) {
    curr++;
    return;
  }
  if (token().newline_before) {
    return;
  }

  std::cerr << token().location << ": expected semicolon or newline, but got "
            << token().type << std::endl;
  std::cerr << HERE << " Location in compiler" << std::endl;
  exit(1);
}

Type *Parser::parse_type() {
  Type *type = nullptr;

  switch (token().type) {
    case TokenType::I32: type = new Type(BaseType::I32); break;
    case TokenType::Bool: type = new Type(BaseType::Bool); break;
    case TokenType::Void: type = new Type(BaseType::Void); break;

    default: UNHANDLED_TYPE();
  }

  ++curr;
  return type;
}

AST *Parser::parse_function() {
  auto node = new AST(ASTType::FunctionDef, token().location);
  consume(TokenType::Def);

  auto name           = consume(TokenType::Identifier);
  node->func_def.name = name.text;

  consume(TokenType::OpenParen);

  // TODO: Add arguments
  auto params = new vector<Variable *>();
  while (!token_is(TokenType::CloseParen)) {
    auto name = consume(TokenType::Identifier);
    consume(TokenType::Colon);
    auto type = parse_type();
    params->push_back(new Variable{name.text, type});
    if (!consume_if(TokenType::Comma)) break;
  }
  node->func_def.params = params;

  consume(TokenType::CloseParen);

  if (consume_if(TokenType::Colon)) {
    node->func_def.return_type = parse_type();
  } else {
    if (name.text == "main") {
      node->func_def.return_type = new Type(BaseType::I32);
    } else {
      node->func_def.return_type = new Type(BaseType::Void);
    }
  }

  node->func_def.body = parse_block();
  return node;
};

AST *Parser::parse_program() {
  auto node = new AST(ASTType::Block, {{}, {}, {}});

  while (!token_is(TokenType::Eof)) {
    switch (token().type) {
      case TokenType::Def: {
        auto func = parse_function();
        node->block.statements->push_back(func);
        break;
      }

      default: UNHANDLED_TYPE();
    }
  };
  return node;
};

AST *Parser::parse_block() {
  auto node = new AST(ASTType::Block, token().location);

  consume(TokenType::OpenCurly);

  while (token().type != TokenType::CloseCurly) {
    auto statement = parse_statement();
    node->block.statements->push_back(statement);
  }

  consume(TokenType::CloseCurly);
  return node;
}

AST *Parser::parse_statement() {
  AST *node = nullptr;

  switch (token().type) {
    case TokenType::If: {
      node = new AST(ASTType::If, token().location);
      consume(TokenType::If);
      node->if_stmt.cond = parse_expression();
      node->if_stmt.body = parse_block();
      if (consume_if(TokenType::Else)) {
        node->if_stmt.els = parse_block();
      } else {
        node->if_stmt.els = nullptr;
      }
      break;
    }
    case TokenType::While: {
      node = new AST(ASTType::While, token().location);
      consume(TokenType::While);
      node->while_loop.cond = parse_expression();
      node->while_loop.body = parse_block();
      break;
    }
    case TokenType::Return: {
      node = new AST(ASTType::Return, token().location);
      consume(TokenType::Return);
      node->unary.expr = parse_expression();
      consume_line_end();
      break;
    }
    case TokenType::Let: {
      node = new AST(ASTType::VarDeclaration, token().location);
      consume(TokenType::Let);

      auto name = consume(TokenType::Identifier);
      consume(TokenType::Colon);
      auto type = parse_type();

      node->var_decl.var = new Variable{name.text, type};

      if (token_is(TokenType::Equals)) {
        consume(TokenType::Equals);
        node->var_decl.init = parse_expression();
      } else {
        node->var_decl.init = nullptr;
      }
      consume_line_end();
      break;
    }
    case TokenType::OpenCurly: {
      node = parse_block();
      break;
    }
    default: {
      node = parse_expression();
      consume_line_end();
      break;
    }
  }

  return node;
}

ASTType token_to_op(TokenType type) {
  switch (type) {
    case TokenType::Plus: return ASTType::Plus;
    case TokenType::Minus: return ASTType::Minus;
    case TokenType::Star: return ASTType::Multiply;
    case TokenType::Slash: return ASTType::Divide;
    case TokenType::LessThan: return ASTType::LessThan;
    case TokenType::GreaterThan: return ASTType::GreaterThan;
    default: break;
  }
  cerr << HERE << " Unhandled token in " << __FUNCTION__ << ": " << type
       << endl;
  exit(1);
}


AST *Parser::parse_factor(bool in_parens) {
  AST *node = nullptr;

  switch (token().type) {
    case TokenType::True: {
      node               = new AST(ASTType::BoolLiteral, token().location);
      node->bool_literal = true;
      consume(TokenType::True);
      break;
    }
    case TokenType::False: {
      node               = new AST(ASTType::BoolLiteral, token().location);
      node->bool_literal = true;
      consume(TokenType::False);
      break;
    }
    case TokenType::IntLiteral: {
      node              = new AST(ASTType::IntLiteral, token().location);
      node->int_literal = token().int_lit;
      consume(TokenType::IntLiteral);
      break;
    }
    case TokenType::StringLiteral: {
      node                 = new AST(ASTType::StringLiteral, token().location);
      node->string_literal = token().text;
      consume(TokenType::StringLiteral);
      break;
    }
    case TokenType::Identifier: {
      node           = new AST(ASTType::Var, token().location);
      node->var.name = consume(TokenType::Identifier).text;
      break;
    }
    case TokenType::OpenParen: {
      consume(TokenType::OpenParen);
      node = parse_expression(true);
      consume(TokenType::CloseParen);
      break;
    }
    default: UNHANDLED_TYPE();
  }

  auto done = false;
  while (!done) {
    if (!in_parens && token().newline_before) break;
    switch (token().type) {
      case TokenType::OpenParen: {
        consume(TokenType::OpenParen);
        auto args = new vector<AST *>();
        while (!token_is(TokenType::CloseParen)) {
          args->push_back(parse_expression());
          if (!consume_if(TokenType::Comma)) break;
        }
        consume(TokenType::CloseParen);
        auto call         = new AST(ASTType::Call, token().location);
        call->call.callee = node;
        call->call.args   = args;
        node              = call;
        break;
      }
      default: done = true;
    }
  }

  return node;
}

AST *Parser::parse_term(bool in_parens) {
  auto lhs = parse_factor(in_parens);
  while (token_is(TokenType::Star) || token_is(TokenType::Slash)) {
    if (!in_parens && token().newline_before) break;
    auto node = new AST(token_to_op(token().type), token().location);
    ++curr;
    node->binary.lhs = lhs;
    node->binary.rhs = parse_term(in_parens);
    lhs              = node;
  }
  return lhs;
}

AST *Parser::parse_additive(bool in_parens) {
  auto lhs = parse_term(in_parens);
  while (token_is(TokenType::Plus) || token_is(TokenType::Minus)) {
    if (!in_parens && token().newline_before) break;
    auto node = new AST(token_to_op(token().type), token().location);
    ++curr;
    node->binary.lhs = lhs;
    node->binary.rhs = parse_term(in_parens);
    lhs              = node;
  }
  return lhs;
}

AST *Parser::parse_relational(bool in_parens) {
  auto lhs = parse_additive(in_parens);
  while (token_is(TokenType::LessThan) || token_is(TokenType::GreaterThan)) {
    if (!in_parens && token().newline_before) break;
    auto node = new AST(token_to_op(token().type), token().location);
    ++curr;
    node->binary.lhs = lhs;
    node->binary.rhs = parse_relational(in_parens);
    lhs              = node;
  }
  return lhs;
}

AST *Parser::parse_expression(bool in_parens) {
  auto lhs = parse_relational(in_parens);

  while (consume_if(TokenType::Equals)) {
    if (!in_parens && token().newline_before) break;
    auto node        = new AST(ASTType::Assignment, token().location);
    node->binary.lhs = lhs;
    node->binary.rhs = parse_expression(in_parens);
    lhs              = node;
  }

  return lhs;
}
