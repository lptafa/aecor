#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#define ENUM_KEYWORDS(F)                                                       \
  F(Def, "def")                                                                \
  F(I32, "i32")                                                                \
  F(Bool, "bool")                                                              \
  F(True, "true")                                                              \
  F(False, "false")                                                            \
  F(If, "if")                                                                  \
  F(Else, "else")                                                              \
  F(Let, "let")                                                                \
  F(Void, "void")                                                              \
  F(While, "while")                                                            \
  F(Return, "return")

#define ENUM_TOKEN_TYPES(F)                                                    \
  F(Identifier, "Identifier")                                                  \
  F(OpenParen, "OpenParen")                                                    \
  F(CloseParen, "CloseParen")                                                  \
  F(Colon, "Colon")                                                            \
  F(Comma, "Comma")                                                            \
  F(Dot, "Dot")                                                                \
  F(Semicolon, "Semicolon")                                                    \
  F(OpenCurly, "OpenCurly")                                                    \
  F(LessThan, "LessThan")                                                      \
  F(GreaterThan, "GreaterThan")                                                \
  F(Equals, "Equals")                                                          \
  F(CloseCurly, "CloseCurly")                                                  \
  F(IntLiteral, "IntLiteral")                                                  \
  F(StringLiteral, "StringLiteral")                                            \
  F(Plus, "Plus")                                                              \
  F(Minus, "Minus")                                                            \
  F(Star, "Star")                                                              \
  F(Slash, "Slash")                                                            \
  F(Ampersand, "Ampersand")                                                    \
  F(Line, "Line")                                                              \
                                                                               \
  F(Eof, "Eof")

#define ENUM_ALL_TOKENS(F)                                                     \
  ENUM_KEYWORDS(F)                                                             \
  ENUM_TOKEN_TYPES(F)

enum class TokenType {
#define F(name, text) name,
  ENUM_ALL_TOKENS(F)
#undef F
};

struct Location {
  std::string_view filename;
  int line;
  int column;
};

struct Token {
  TokenType type;
  Location location;
  std::string_view text;
  int int_lit;
  bool newline_before = false;

  Token() {}

  static Token from_type(TokenType type, Location location,
                         std::string_view text = {});
  static Token from_name(std::string_view name, Location location);
  static Token from_int_literal(int value, Location location);
};

inline std::ostream &operator<<(std::ostream &os, const Location &loc) {
  os << loc.filename << ":" << loc.line << ":" << loc.column;
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const TokenType &type) {
  switch (type) {
#define F(name, keyword)                                                       \
  case TokenType::name: os << keyword; break;
    ENUM_ALL_TOKENS(F)
#undef F
  }
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const Token &tok) {
  os << "Token: " << tok.type;
  os << ", " << tok.location;
  if (!tok.text.empty()) { os << ", (" << tok.text << ")"; }
  return os;
}
