#ifndef TOKEN_H
#define TOKEN_H
#include "module.hpp"
#include <fmt/format.h>

struct token {
  enum type {
    IDENT,
    INT,
    FLOAT,
    DEF,
    EXTERN,
    TOK_EOF,
    INVALID,
  };

  type kind;
  module::span loc;

  token() = default;
  token(type kind, module::span loc) : kind(kind), loc(std::move(loc)) {}

  std::string_view lexeme() const { return loc.contents(); }

};


template <> struct fmt::formatter<token>: formatter<string_view> {
  template <typename FormatContext>
  auto format(const token& tok, FormatContext& ctx) {
    const char* kind;
    switch (tok.kind) {
      case token::type::IDENT: kind = "IDENTIFIER"; break;
      case token::type::INT: kind = "INT"; break;
      case token::type::FLOAT: kind = "FLOAT"; break;
      case token::type::DEF: kind = "DEF"; break;
      case token::type::EXTERN: kind = "EXTERN"; break;
      case token::type::TOK_EOF: kind = "EOF"; break;
      case token::type::INVALID: kind = "INVALID TOKEN"; break;
    }
    return
      (formatter<string_view>::format
        (fmt::format("{:<13} '{}'", kind, tok.lexeme()), ctx));
  }
};

#endif
