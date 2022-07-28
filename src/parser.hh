#pragma once

#include <ast.hh>
#include <locale>
#include <string>
#include <string_view>
#include <tokens.hh>
#include <types.hh>
#include <utils.hh>
#include <vector>

using namespace std;

struct Parser {
  vector<Token> tokens;
  int curr = 0;

  Type *parse_type();

  AST *parse_factor(bool in_parens);
  AST *parse_term(bool in_parens);
  AST *parse_additive(bool in_parens);
  AST *parse_relational(bool in_parens);
  AST *parse_expression(bool in_parens = false);

  AST *parse_program();
  AST *parse_function();
  AST *parse_block();
  AST *parse_statement();

  Token &consume_impl(TokenType token_type, const char *sloc);
  Token &expect_impl(TokenType token_type, const char *sloc);

  bool consume_if(TokenType token_type);

  bool token_is(TokenType token_type) { return token().type == token_type; }
  void consume_line_end();

  Parser(vector<Token> tokens) : tokens(tokens) {}
  Token &token() { return tokens[curr]; };
};

#define consume(token_type) consume_impl(token_type, HERE)
#define expect(token_type) expect_impl(token_type, HERE)
