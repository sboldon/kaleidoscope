#include "lexer.hpp"
#include "error.hpp"
#include "keyhash.hpp"
#include "lexer_patterns.hpp"
#include "doctest.h"
#include <optional>
#include <fmt/core.h>
#include <fmt/compile.h>


template <lexable_source T> char lexer<T>::peek() const { return *current; }

// Checking to make sure that *current != EOF is not necessary because the
// source has an additional null byte appended to it.
template <lexable_source T> char lexer<T>::peek_next() const {
  return current[1];
}

template <lexable_source T> char lexer<T>::next() { return *current++; }

template <lexable_source T> token lexer<T>::make_token(token::type kind) const {
  return token(kind, module::span(start, current));
}

template <lexable_source T> void lexer<T>::mark_error(error_type kind) {
  // errors.push_back(source.make_error(msg, module::span(start, current)));
  source.mark_error(kind, module::span(current, current + 1));
}

template <lexable_source T> void lexer<T>::consume_whitespace() {
  for (;;) {
    switch (peek()) {
    case '/':
      // Handle line comments.
      if (peek_next() == '/') {
        current += 2;
        while (peek() != '\n' && peek() != '\0') {
          next();
        };
      } else {
        return;
      }
      // Fallthrough enabled because the current character is either '\n' or '\0'. It is useful to
      // have the address of the second EOF byte in `line_offsets` because it removes an edge case;
      // the address of the last character in any given line can be calculated with:
      // `line_offsets[<line-number>] - 2`.
    case '\n':
      line_offsets.push_back(current + 1);
    case ' ':
    case '\t':
    case '\r':
    case '\f':
    case '\v':
      next();
      break;
    default:
      return;
    }
  }
}

// Advance scanner while there is a valid identifier character.
template <lexable_source T> void lexer<T>::scan_ident_chars() {
  for (;;) {
    switch (peek()) {
    ALPHA
    DECIMAL
    case '_':
      next();
      break;
    default:
      return;
    }
  }
}

template <lexable_source T> token lexer<T>::seen_keyword_char() {
  scan_ident_chars();
  const auto loc = module::span(start, current);
  return token(perfect_hash::get_token(start, loc.len()), loc);
}

// Discard any further errors regarding the same literal after encountering an invalid digit.
template <lexable_source T>
void lexer<T>::consume_invalid_num_lit(/*std::string reason*/) {
  for (;;) {
    switch (peek()) {
    case 'e':
    case 'p':
    case 'E':
    case 'P': {
      // Scientific notation.
      char ch = next();
      if (ch == '-' || ch == '+') {
        next();
      }
      break;
    }
    case '_':
    case '.':
    ALPHA_NON_SCI
    DECIMAL
      next();
      break;
    default:
      return;
    }
  }
}

// To enhance readability, digits may be separated by an underscore.
#define OPTIONAL_DIGIT_SEP                                                                         \
  case '_':                                                                                        \
    next();                                                                                        \
    switch (peek()) {                                                                              \
    DECIMAL                                                                                        \
      next();                                                                                      \
      break;                                                                                       \
    default:                                                                                       \
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_dec_digit});         \
      consume_invalid_num_lit();                                                                   \
      return token::type::INVALID;                                                                 \
    }

// At this point, a radix point has already been seen and any additional '.' characters result in an
// invalid literal.
template <lexable_source T> token::type lexer<T>::scan_dec_digits() {
  for (;;) {
    switch (peek()) {
    // Valid literal character.
    DECIMAL
      next();
      break;
    OPTIONAL_DIGIT_SEP
      break;
    // Invalid literal character.
    case '.':
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::multiple_radix_points});
      consume_invalid_num_lit();
      return token::type::INVALID;
    ALPHA
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_dec_digit});
      consume_invalid_num_lit();
      return token::type::INVALID;
    // End of valid literal.
    default:
      return token::type::FLOAT;
    }
  }
}

// Hexadecimal floats written in scientific notation still have a decimal exponent.
template <lexable_source T> token::type lexer<T>::seen_exponent_char() {
  switch (peek()) {
  DECIMAL
  case '+':
  case '-':
    next();
    return scan_dec_digits();
  ALPHA
    // TODO: Is it more helpful to have this error specific to exponent?
    // "error: invalid numeric literal: expected '+', '-', or a decimal digit"
    mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_dec_digit});
    consume_invalid_num_lit();
    return token::type::INVALID;
  default:
    mark_error({.tag = error_type::invalid_num_lit, .info = error_type::missing_exponent});
    return token::type::INVALID;
  }
}

template <lexable_source T> token::type lexer<T>::seen_dec_point() {
  switch (peek()) {
    DECIMAL
    next();
    for (;;) {
      switch (peek()) {
      DECIMAL
        next();
        break;
      OPTIONAL_DIGIT_SEP
        break;
      case 'e':
      case 'E':
        next();
        return seen_exponent_char();
      case '.':
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::multiple_radix_points});
        consume_invalid_num_lit();
        return token::type::INVALID;
      ALPHA_NO_E
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_dec_digit});
        consume_invalid_num_lit();
        return token::type::INVALID;
      default:
        return token::type::FLOAT;
      }
    }
  default:
    mark_error({.tag = error_type::invalid_num_lit, .info = error_type::missing_fraction_part});
    consume_invalid_num_lit();
    return token::type::INVALID;
  }
}

template <lexable_source T> token::type lexer<T>::seen_dec_digit() {
  for (;;) {
    switch (peek()) {
    DECIMAL
      next();
      break;
    OPTIONAL_DIGIT_SEP
      break;
    case '.':
      next();
      return seen_dec_point();
    case 'e':
    case 'E':
      next();
      return seen_exponent_char();
    ALPHA_NO_E
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_dec_digit});
      consume_invalid_num_lit();
      return token::type::INVALID;
    default:
      return token::type::INT;
    }
  }
}

// Any non-binary alphanumeric character results in an invalid token.
template <lexable_source T> token::type lexer<T>::seen_bin_lit_prefix() {
  for (;;) {
    switch (peek()) {
    case '0':
    case '1':
      next();
      break;
    case '_':
      next();
      switch(peek()) {
      case '0':
      case '1':
        next();
        break;
      default:
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_bin_digit});
        consume_invalid_num_lit();
        return token::type::INVALID;
      }
      break;
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    ALPHA
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_bin_digit});
      consume_invalid_num_lit();
      return token::type::INVALID;
    default:
      return token::type::INT;
    }
  }
}

// Any non-octal alphanumeric character results in an invalid token.
template <lexable_source T> token::type lexer<T>::seen_oct_lit_prefix() {
  for (;;) {
    switch (peek()) {
    OCTAL
      next();
      break;
    case '_':
      next();
      switch(peek()) {
      OCTAL
        next();
        break;
      default:
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_oct_digit});
        consume_invalid_num_lit();
        return token::type::INVALID;
      }
      break;
    case '8':
    case '9':
    ALPHA
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_oct_digit});
      consume_invalid_num_lit();
      return token::type::INVALID;
    default:
      return token::type::INT;
    }
  }
}

template <lexable_source T> token::type lexer<T>::seen_hex_point() {
  switch (peek()) {
    HEX next();
    for (;;) {
      switch (peek()) {
      HEX
        next();
        break;
      case '_':
        next();
        switch(peek()) {
        HEX
          next();
          break;
        default:
          mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_hex_digit});
          consume_invalid_num_lit();
          return token::type::INVALID;
        }
        break;
      case 'p':
      case 'P':
        next();
        return seen_exponent_char();
      case '.':
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::multiple_radix_points});
        consume_invalid_num_lit();
        return token::type::INVALID;
        break;
      ALPHA_NON_HEX_NO_P
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_hex_digit});
        consume_invalid_num_lit();
        return token::type::INVALID;
      default:
        return token::type::FLOAT;
      }
    }
  default:
    mark_error({.tag = error_type::invalid_num_lit, .info = error_type::missing_fraction_part});
    return token::type::INVALID;
  }
}

// Any non-hex alphanumeric character results in an invalid token.
template <lexable_source T> token::type lexer<T>::seen_hex_lit_prefix() {
  for (;;) {
    switch (peek()) {
    HEX
      next();
      break;
    case '_':
      next();
      switch(peek()) {
      HEX
        next();
        break;
      default:
        mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_hex_digit});
        consume_invalid_num_lit();
        return token::type::INVALID;
      }
      break;
    case '.':
      next();
      return seen_hex_point();
    case 'p':
    case 'P':
      next();
      return seen_exponent_char();
    ALPHA_NON_HEX_NO_P
      mark_error({.tag = error_type::invalid_num_lit, .info = error_type::non_hex_digit});
      consume_invalid_num_lit();
      return token::type::INVALID;
    default:
      return token::type::INT;
    }
  }
}

template <lexable_source T> token::type lexer<T>::seen_zero() {
  switch (peek()) {
  DECIMAL
    next();
    return seen_dec_digit();
  OPTIONAL_DIGIT_SEP
    return seen_dec_digit();
  case 'b':
  case 'B':
    next();
    return seen_bin_lit_prefix();
  case 'o':
  case 'O':
    next();
    return seen_oct_lit_prefix();
  case 'x':
  case 'X':
    next();
    return seen_hex_lit_prefix();
  case '.':
    next();
    return seen_dec_point();
  ALPHA_NON_RADIX
    mark_error({.tag = error_type::invalid_num_lit, .info = error_type::unknown_radix_prefix});
    consume_invalid_num_lit();
    return token::type::INVALID;
  default:
    return token::type::INT;
  }
}

template <lexable_source T> token lexer<T>::next_token() {
  consume_whitespace();
  start = current;
  switch (next()) {
  case '\0':
    return make_token(token::type::TOK_EOF);
  case 'a':
  case 'b':
  case 'c':
    scan_ident_chars();
    return make_token(token::type::IDENT);
  case 'd':
  case 'e':
    // DEF, EXTERN
    return seen_keyword_char();
  case 'f':
  case 'g':
  case 'h':
  case 'i':
  case 'j':
  case 'k':
  case 'l':
  case 'm':
  case 'n':
  case 'o':
  case 'p':
  case 'q':
  case 'r':
  case 's':
  case 't':
  case 'u':
  case 'v':
  case 'w':
  case 'x':
  case 'y':
  case 'z':
    UPPER_ALPHA
  case '_':
    scan_ident_chars();
    return make_token(token::type::IDENT);
  case '0':
    return make_token(seen_zero());
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    return make_token(seen_dec_digit());
  default:
    source.mark_error({.tag = error_type::unknown_char, .ch = *start}, module::span(start, current));
    return make_token(token::type::INVALID);
  }
}


// Explicit instantiation of `lexer<module::file>` allows lexer<T> implementation to be separate
// from its declaration.
namespace module {
  struct file;
}
template class lexer<module::file>;


// Unit tests
TEST_CASE("lexer<T>::next_token") {
  using enum token::type;
  using enum error_type::reason;
  using enum error_type::detail;

  // Satisfies lexable_source concept and tracks errors for testing of error handling.
  struct test_source {
    std::string contents;
    std::vector<const char*> line_offsets;
    std::optional<error_type> err_reason;

    test_source(const char* buf) : contents(buf) {
      contents.push_back('\0');
      line_offsets.push_back(contents.c_str());
    }

    const char* start() { return contents.data(); }

    bool has_error() { return err_reason.has_value(); }

    void mark_error(error_type kind, const module::span& loc) {
      err_reason = kind;
    }
  };

  char subcase_name[256];

#define CHECK_TOK(ty, text)                                                                        \
  fmt::format_to(subcase_name, FMT_COMPILE("`{}`\0"), text);                                       \
  SUBCASE(subcase_name) {                                                                          \
    auto src = test_source(text);                                                                  \
    auto lex = lexer(src);                                                                         \
    auto tok = lex.next_token();                                                                   \
    CHECK(tok.kind == ty);                                                                         \
    CHECK(tok.lexeme() == text);                                                                   \
    CHECK(!src.has_error());                                                                       \
  }

#define CHECK_ERR(cause, text)                                                                     \
  fmt::format_to(subcase_name, FMT_COMPILE("`{}`\0"), text);                                       \
  SUBCASE(subcase_name) {                                                                          \
    auto src = test_source(text);                                                                  \
    auto lex = lexer(src);                                                                         \
    lex.next_token();                                                                              \
    REQUIRE(src.has_error());                                                                      \
    CHECK(src.err_reason->eq(cause));                                                              \
  }

// #define CHECK_TOK_LEXEME(ty, expect, text)                                                         \
//   fmt::format_to(subcase_name, FMT_COMPILE("`{}`\0"), expect);                                     \
//   SUBCASE(subcase_name) {                                                                          \
//     auto f = module::file(text);                                                                   \
//     auto lex = lexer(f);                                                                           \
//     auto tok = lex.next_token();                                                                   \
//     CHECK(tok.kind == ty);                                                                         \
//     CHECK(tok.lexeme() == expect);                                                                 \
//     CHECK(!f.has_error());                                                                         \
//   }

  // Keywords & identifiers //
  CHECK_TOK(DEF, "def");
  CHECK_TOK(EXTERN, "extern");
  CHECK_TOK(IDENT, "_def");
  CHECK_TOK(IDENT, "deff");

  // Numeric literals //
  CHECK_TOK(INT, "0");
  CHECK_TOK(INT, "0b0");
  CHECK_TOK(INT, "0o0");
  CHECK_TOK(INT, "0x0");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = unknown_radix_prefix}), "0t0");
  CHECK_TOK(INT, "000");
  CHECK_TOK(INT, "0_0");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "0_0_");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "0_a");
  CHECK_TOK(FLOAT, "0.0");
  CHECK_TOK(INT, "1");
  CHECK_TOK(INT, "1234567890");
  CHECK_TOK(INT, "1_2_3_4");
  CHECK_TOK(FLOAT, "1.25");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = missing_fraction_part}), "1._25");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = multiple_radix_points}), "1.25.98");
  CHECK_TOK(FLOAT, "1.2_5");
  CHECK_TOK(FLOAT, "40e9");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "40f9");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = missing_exponent}), "40e_123");
  CHECK_TOK(FLOAT, "40E9");
  CHECK_TOK(FLOAT, "40e+9");
  CHECK_TOK(FLOAT, "40e-9");
  CHECK_TOK(FLOAT, "40e9");
  CHECK_TOK(FLOAT, "40E9");
  CHECK_TOK(FLOAT, "40.20e9");
  CHECK_TOK(FLOAT, "40.20e+9");
  CHECK_TOK(FLOAT, "40.20e-9");
  CHECK_TOK(FLOAT, "100_024.2_0E021");
  CHECK_TOK(INT, "0b01");
  CHECK_TOK(INT, "0b_01");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_bin_digit}), "0b012");
  CHECK_TOK(INT, "0b_0000_0100");
  CHECK_TOK(INT, "0o777");
  CHECK_TOK(INT, "0o_777");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_oct_digit}), "0o7778");
  CHECK_TOK(INT, "0o01234567");
  CHECK_TOK(INT, "0xfF");
  CHECK_TOK(INT, "0x0000_FFFF");
  CHECK_TOK(INT, "0x_dead_beef");
  CHECK_TOK(INT, "0x_DEAD_BEEF");
  CHECK_TOK(INT, "0xabcdefABCDEF012345689");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = non_hex_digit}), "0xabcdefABCDEFg012");
  CHECK_TOK(FLOAT, "0x0p-12");
  CHECK_TOK(FLOAT, "0x0P+12");
  CHECK_TOK(FLOAT, "0x0P12");
  CHECK_TOK(FLOAT, "0x1.921fb54442d18p+0001");
  CHECK_ERR((error_type{.tag = invalid_num_lit, .info = missing_fraction_part}), "0xffaa._2139432");

#undef CHECK_TOK
#undef CHECK_ERR
}
