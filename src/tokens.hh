#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#define ENUM_KEYWORDS(F)                                                       \
  F(And, "and")                                                                \
  F(As, "as")                                                                  \
  F(Bool, "bool")                                                              \
  F(Char, "char")                                                              \
  F(Def, "def")                                                                \
  F(Defer, "defer")                                                            \
  F(Else, "else")                                                              \
  F(Extern, "extern")                                                          \
  F(False, "false")                                                            \
  F(F32, "f32")                                                                \
  F(For, "for")                                                                \
  F(Fn, "fn")                                                                  \
  F(I32, "i32")                                                                \
  F(If, "if")                                                                  \
  F(Let, "let")                                                                \
  F(Not, "not")                                                                \
  F(Or, "or")                                                                  \
  F(Return, "return")                                                          \
  F(String, "string")                                                          \
  F(Struct, "struct")                                                          \
  F(True, "true")                                                              \
  F(U8, "u8")                                                                  \
  F(UnypedPtr, "untyped_ptr")                                                  \
  F(Use, "use")                                                                \
  F(Void, "void")                                                              \
  F(While, "while")

#define ENUM_TOKEN_TYPES(F)                                                    \
  F(AtSign, "@")                                                               \
  F(Ampersand, "Ampersand")                                                    \
  F(CloseCurly, "CloseCurly")                                                  \
  F(CloseParen, "CloseParen")                                                  \
  F(CloseSquare, "CloseSquare")                                                \
  F(Colon, "Colon")                                                            \
  F(ColonColon, "ColonColon")                                                  \
  F(Comma, "Comma")                                                            \
  F(Dot, "Dot")                                                                \
  F(Eof, "Eof")                                                                \
  F(Equals, "Equals")                                                          \
  F(EqualEquals, "EqualEquals")                                                \
  F(Exclamation, "Exclamation")                                                \
  F(FloatLiteral, "FloatLiteral")                                              \
  F(GreaterThan, "GreaterThan")                                                \
  F(GreaterThanEquals, "GreaterThanEquals")                                    \
  F(Identifier, "Identifier")                                                  \
  F(IntLiteral, "IntLiteral")                                                  \
  F(LessThan, "LessThan")                                                      \
  F(LessThanEquals, "LessThanEquals")                                          \
  F(Line, "Line")                                                              \
  F(Minus, "Minus")                                                            \
  F(MinusEquals, "MinusEquals")                                                \
  F(NotEquals, "NotEquals")                                                    \
  F(OpenCurly, "OpenCurly")                                                    \
  F(OpenParen, "OpenParen")                                                    \
  F(OpenSquare, "OpenSquare")                                                  \
  F(Plus, "Plus")                                                              \
  F(PlusEquals, "PlusEquals")                                                  \
  F(Semicolon, "Semicolon")                                                    \
  F(Slash, "Slash")                                                            \
  F(SlashEquals, "SlashEquals")                                                \
  F(Star, "Star")                                                              \
  F(StarEquals, "StarEquals")                                                  \
  F(StringLiteral, "StringLiteral")

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
