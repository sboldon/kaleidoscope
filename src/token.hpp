#ifndef TOKEN_H
#define TOKEN_H
#include "module.hpp"
#include <fmt/format.h>

struct token {
  enum type : uint8_t {
    ident,
    int_literal,
    float_literal,

    plus,
    dash,
    star,
    fwd_slash,
    bang,

    keyword_def,
    keyword_extern,

    left_paren,
    right_paren,

    eof,
    invalid,

    num_tokens // Must stay at end of enum.
  };

  type kind;
  module::span loc;

  token() = default;
  token(type kind, module::span loc) : kind(kind), loc(std::move(loc)) {}

  std::string_view lexeme() const { return loc.contents(); }

  static const char* category(type kind) {
    switch (kind) {
      case ident: return "an identifier";
      case int_literal:
      case float_literal:
        return "a literal";
      case plus:
      case dash:
      case star:
      case fwd_slash:
      case bang:
        return "an operator";
      case keyword_def:
      case keyword_extern:
        return "a keyword";
      case left_paren: return "'('";
      case right_paren: return "')'";
      case eof: return "end of file";
      default: return ""; // Unused.
    }
  }
};


template <> struct fmt::formatter<token>: formatter<string_view> {
  template <typename FormatContext>
  auto format(const token& tok, FormatContext& ctx) {
    using enum token::type;
    const char* str;
    switch (tok.kind) {
      case ident: str = "IDENTIFIER"; break;
      case int_literal: str = "INT LITERAL"; break;
      case float_literal: str = "FLOAT LITERAL"; break;
      case plus: str = "PLUS"; break;
      case dash: str = "DASH"; break;
      case star: str = "STAR"; break;
      case fwd_slash: str = "FWD SLASH"; break;
      case bang: str = "BANG"; break;
      case keyword_def: str = "DEF"; break;
      case keyword_extern: str = "EXTERN"; break;
      case left_paren: str = "LEFT PAREN"; break;
      case right_paren: str = "RIGHT PAREN"; break;
      case eof: str = "EOF"; break;
      default: str = "INVALID TOKEN"; break;
    }
    return
      (formatter<string_view>::format
        (fmt::format("{:<13} '{}'", str, tok.lexeme()), ctx));
  }
};

#endif
