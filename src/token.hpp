#ifndef TOKEN_H
#define TOKEN_H
#include "location.hpp"

struct Token {
  enum Type {
    IDENT,
    NUM,
    DEF,
    EXTERN,
    TOK_EOF,
    INVALID,
  };

  Type kind;
  Span loc;

  Token() = default;
  Token(Type kind, Span loc) : kind(kind), loc(std::move(loc)) {}

  std::string_view lexeme() const { return loc.contents(); }
};

#endif
