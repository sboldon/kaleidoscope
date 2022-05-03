#include "lexer.hpp"
#include "lexer_patterns.hpp"
#include "error.hpp"
#include "keyhash.hpp"
#include "parser.hpp"
#include "doctest.hpp"

#include <optional>
#include <fmt/core.h>
#include <fmt/compile.h>

template <lexable T> char lexer<T>::peek() const { return *current; }

// Checking to make sure that *current != EOF is not necessary because the
// source has an additional null byte appended to it.
template <lexable T> char lexer<T>::peek_next() const {
  return current[1];
}

template <lexable T> char lexer<T>::next() { return *current++; }

template <lexable T> token lexer<T>::make_token(token::type kind) const {
  return token(kind, module::span(start, current));
}

template <lexable T> void lexer<T>::mark_error(error_type kind) {
  source.mark_error(std::move(kind), module::span(current, current + 1));
}

template <lexable T> void lexer<T>::consume_whitespace() {
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
template <lexable T> void lexer<T>::scan_ident_chars() {
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

template <lexable T> token lexer<T>::seen_keyword_char() {
  scan_ident_chars();
  const auto loc = module::span(start, current);
  return token(perfect_hash::get_token(start, loc.len()), loc);
}

// Discard any further errors regarding the same literal after encountering an invalid digit.
template <lexable T>
void lexer<T>::consume_invalid_num_lit(error_type::detail cause) {
  mark_error({.tag = error_type::reason::invalid_num_lit, .info = cause});
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
      DECIMAL                                                                                      \
        next();                                                                                    \
        break;                                                                                     \
      default:                                                                                     \
        consume_invalid_num_lit(error_type::detail::non_dec_digit);                                \
        return token::type::invalid;                                                               \
    }

// At this point, a radix point has already been seen and any additional '.' characters result in an
// invalid literal.
template <lexable T> token::type lexer<T>::scan_dec_digits() {
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
        consume_invalid_num_lit(error_type::detail::multiple_radix_points);
        return token::type::invalid;
      ALPHA
        consume_invalid_num_lit(error_type::detail::non_dec_digit);
        return token::type::invalid;
      // End of valid literal.
      default:
        return token::type::float_literal;
    }
  }
}

// Hexadecimal floats written in scientific notation still have a decimal exponent.
template <lexable T> token::type lexer<T>::seen_exponent_char() {
  switch (peek()) {
    DECIMAL
    case '+':
    case '-':
      next();
      return scan_dec_digits();
    ALPHA
      // TODO: Is it more helpful to have this error specific to exponent?
      // "error: invalid numeric literal: expected '+', '-', or a decimal digit"
      consume_invalid_num_lit(error_type::detail::non_dec_digit);
      return token::type::invalid;
    default:
      consume_invalid_num_lit(error_type::detail::missing_exponent);
      return token::type::invalid;
  }
}

template <lexable T> token::type lexer<T>::seen_dec_point() {
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
            consume_invalid_num_lit(error_type::detail::multiple_radix_points);
            return token::type::invalid;
          ALPHA_NO_E
            consume_invalid_num_lit(error_type::detail::non_dec_digit);
            return token::type::invalid;
          default:
            return token::type::float_literal;
        }
      }
    default:
      consume_invalid_num_lit(error_type::detail::missing_fraction_part);
      return token::type::invalid;
  }
}

template <lexable T> token::type lexer<T>::seen_dec_digit() {
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
        consume_invalid_num_lit(error_type::detail::non_dec_digit);
        return token::type::invalid;
      default:
        return token::type::int_literal;
    }
  }
}

// Any non-binary alphanumeric character results in an invalid token.
template <lexable T> token::type lexer<T>::seen_bin_lit_prefix() {
  for (;;) {
    switch (peek()) {
      case '0':
      case '1':
        next();
        break;
      case '_':
        next();
        switch (peek()) {
          case '0':
          case '1':
            next();
            break;
          default:
            consume_invalid_num_lit(error_type::detail::non_bin_digit);
            return token::type::invalid;
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
        consume_invalid_num_lit(error_type::detail::non_bin_digit);
        return token::type::invalid;
      default:
        return token::type::int_literal;
    }
  }
}

// Any non-octal alphanumeric character results in an invalid token.
template <lexable T> token::type lexer<T>::seen_oct_lit_prefix() {
  for (;;) {
    switch (peek()) {
      OCTAL
        next();
        break;
      case '_':
        next();
        switch (peek()) {
          OCTAL
            next();
            break;
          default:
            consume_invalid_num_lit(error_type::detail::non_oct_digit);
            return token::type::invalid;
        }
        break;
      case '8':
      case '9':
      ALPHA
        consume_invalid_num_lit(error_type::detail::non_oct_digit);
        return token::type::invalid;
      default:
        return token::type::int_literal;
    }
  }
}

template <lexable T> token::type lexer<T>::seen_hex_point() {
  switch (peek()) {
    HEX
      next();
      for (;;) {
        switch (peek()) {
          HEX
            next();
            break;
          case '_':
            next();
            switch (peek()) {
              HEX
                next();
                break;
              default:
                consume_invalid_num_lit(error_type::detail::non_hex_digit);
                return token::type::invalid;
            }
            break;
          case 'p':
          case 'P':
            next();
            return seen_exponent_char();
          case '.':
            consume_invalid_num_lit(error_type::detail::multiple_radix_points);
            return token::type::invalid;
            break;
          ALPHA_NON_HEX_NO_P
            consume_invalid_num_lit(error_type::detail::non_hex_digit);
            return token::type::invalid;
          default:
            return token::type::float_literal;
        }
      }
    default:
      consume_invalid_num_lit(error_type::detail::missing_fraction_part);
      return token::type::invalid;
  }
}

// Any non-hex alphanumeric character results in an invalid token.
template <lexable T> token::type lexer<T>::seen_hex_lit_prefix() {
  for (;;) {
    switch (peek()) {
      HEX
        next();
        break;
      case '_':
        next();
        switch (peek()) {
          HEX
            next();
            break;
          default:
            consume_invalid_num_lit(error_type::detail::non_hex_digit);
            return token::type::invalid;
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
        consume_invalid_num_lit(error_type::detail::non_hex_digit);
        return token::type::invalid;
      default:
        return token::type::int_literal;
    }
  }
}

template <lexable T> token::type lexer<T>::seen_zero() {
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
      consume_invalid_num_lit(error_type::detail::unknown_radix_prefix);
      return token::type::invalid;
    default:
      return token::type::int_literal;
  }
}

template <lexable T> token lexer<T>::next_token() {
  consume_whitespace();
  start = current;
  switch (next()) {
    case '\0':
      return make_token(token::type::eof);
    case 'a':
    case 'b':
    case 'c':
      scan_ident_chars();
      return make_token(token::type::ident);
    case 'd':
    case 'e':
      // `def`, `extern`
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
    case '_':
    UPPER_ALPHA
      scan_ident_chars();
      return make_token(token::type::ident);
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
    case '+':
      return make_token(token::type::plus);
    case '-':
      return make_token(token::type::dash);
    case '*':
      return make_token(token::type::star);
    case '/':
      return make_token(token::type::fwd_slash);
    case '!':
      return make_token(token::type::bang);
    case '(':
      return make_token(token::type::left_paren);
    case ')':
      return make_token(token::type::right_paren);
    default:
      (source.mark_error
        ({.tag = error_type::reason::unknown_char, .ch = *start},
         module::span(start, current)));
      return make_token(token::type::invalid);
  }
}


// Declaring the specific `lexable` types used with `lexer<T>` allows the implementation of
// `lexer<T>` to be separate from its declaration.
template class lexer<module::file>;
#if !defined(DOCTEST_CONFIG_DISABLE)
template class lexer<parsing::parser_test_source>;
#endif


//------------------------------------------------------------------------------------------------//
#if !defined(DOCTEST_CONFIG_DISABLE)
lexer_test_source::lexer_test_source(const char* buf) : contents(buf) {
    contents.push_back('\0');
    line_offsets.push_back(contents.c_str());
}

const char* lexer_test_source::start() const { return contents.data(); }

bool lexer_test_source::has_error() const { return err_reason.has_value(); }

void lexer_test_source::mark_error(error_type kind, const module::span& loc) {
  err_reason = kind;
}

namespace {
  void test(token::type expected, const char* text) {
    CAPTURE(text);
    lexer_test_source src = lexer_test_source(text);
    lexer lex = lexer(src);
    auto tok = lex.next_token();
    CHECK(tok.kind == expected);
    CHECK(tok.lexeme() == text);
    CHECK(!src.has_error());
  }

  void test_err(error_type expected, const char* text) {
    CAPTURE(text);
    lexer_test_source src = lexer_test_source(text);
    lexer lex = lexer(src);
    lex.next_token();
    REQUIRE(src.has_error());
    CHECK(src.err_reason == expected);
  }
} // End unnamed namespace.


TEST_SUITE_BEGIN("lexing");
using enum token::type;
using enum error_type::reason;
using enum error_type::detail;

TEST_CASE("operators") {
  test(plus, "+");
  test(dash, "-");
  test(star, "*");
  test(fwd_slash, "/");
  test(bang, "!");
}

TEST_CASE("punctuators") {
  test(left_paren, "(");
  test(right_paren, ")");
}

TEST_CASE("keywords & identifiers") {
  test(keyword_def, "def");
  test(keyword_extern, "extern");
  test(ident, "_def");
  test(ident, "deff");
}

TEST_CASE("decimal int literals") {
  test(int_literal, "0");
  test(int_literal, "000");
  test(int_literal, "0_0");
  test_err((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "0_0_");
  test_err((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "0_a");
  test(int_literal, "1");
  test(int_literal, "2");
  test(int_literal, "3");
  test(int_literal, "4");
  test(int_literal, "5");
  test(int_literal, "6");
  test(int_literal, "7");
  test(int_literal, "8");
  test(int_literal, "9");
  test(int_literal, "1234567890");
  test(int_literal, "1_2_3_4");
}

TEST_CASE("binary int literals") {
  test(int_literal, "0b0");
  test_err((error_type{.tag = invalid_num_lit, .info = unknown_radix_prefix}), "0t0");
  test(int_literal, "0b_0");
  test(int_literal, "0b1");
  test(int_literal, "0b01");
  test(int_literal, "0b_01");
  test_err((error_type{.tag = invalid_num_lit, .info = non_bin_digit}), "0b012");
  test(int_literal, "0b_0000_0100");
}

TEST_CASE("octal int literals") {
  test(int_literal, "0o0");
  test(int_literal, "0o_0");
  test(int_literal, "0o1");
  test(int_literal, "0o2");
  test(int_literal, "0o3");
  test(int_literal, "0o4");
  test(int_literal, "0o5");
  test(int_literal, "0o6");
  test(int_literal, "0o7");
  test(int_literal, "0o777");
  test(int_literal, "0o_777");
  test_err((error_type{.tag = invalid_num_lit, .info = non_oct_digit}), "0o7778");
}

TEST_CASE("hex int literals") {
  test(int_literal, "0x0");
  test(int_literal, "0x1");
  test(int_literal, "0x2");
  test(int_literal, "0x3");
  test(int_literal, "0x4");
  test(int_literal, "0x5");
  test(int_literal, "0x6");
  test(int_literal, "0x7");
  test(int_literal, "0x8");
  test(int_literal, "0x9");
  test(int_literal, "0xa");
  test(int_literal, "0xb");
  test(int_literal, "0xc");
  test(int_literal, "0xd");
  test(int_literal, "0xe");
  test(int_literal, "0xf");
  test(int_literal, "0xA");
  test(int_literal, "0xB");
  test(int_literal, "0xC");
  test(int_literal, "0xD");
  test(int_literal, "0xE");
  test(int_literal, "0xF");
  test(int_literal, "0o01234567");
  test(int_literal, "0xfF");
  test(int_literal, "0x0000_FFFF");
  test(int_literal, "0x_dead_beef");
  test(int_literal, "0x_DEAD_BEEF");
  test(int_literal, "0xabcdefABCDEF012345689");
  test_err((error_type{.tag = invalid_num_lit, .info = non_hex_digit}), "0xabcdefABCDEFg012");
  test(int_literal, "0x40e9");
}

TEST_CASE("decimal float literals") {
  test(float_literal, "0.0");
  test(float_literal, "1.25");
  test_err((error_type{.tag = invalid_num_lit, .info = missing_fraction_part}), "1._25");
  test_err((error_type{.tag = invalid_num_lit, .info = multiple_radix_points}), "1.25.98");
  test(float_literal, "1.2_5");
  test(float_literal, "40e9");
  test_err((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "40f9");
  test_err((error_type{.tag = invalid_num_lit, .info = missing_exponent}), "40e_123");
  test(float_literal, "40E9");
  test(float_literal, "40e+9");
  test(float_literal, "40e-9");
  test(float_literal, "40e9");
  test(float_literal, "40E9");
  test(float_literal, "40.20e9");
  test(float_literal, "40.20e+9");
  test(float_literal, "40.20e-9");
  test(float_literal, "100_024.2_0E021");
}

TEST_CASE("hex float literals") {
  test(float_literal, "0x0.0");
  test(float_literal, "0x1.25");
  test_err((error_type{.tag = invalid_num_lit, .info = missing_fraction_part}), "0x1._25");
  test_err((error_type{.tag = invalid_num_lit, .info = multiple_radix_points}), "0x1.25.98");
  test(float_literal, "0x1.2_5");
  test(float_literal, "0x40p9");
  test_err((error_type{.tag = invalid_num_lit, .info = non_dec_digit}), "40f9");
  test_err((error_type{.tag = invalid_num_lit, .info = missing_exponent}), "40e_123");
  test(float_literal, "0x0p-12");
  test(float_literal, "0x0P+12");
  test(float_literal, "0x0P12");
  test(float_literal, "0x1.921fb54442d18p+0001");
  test_err((error_type{.tag = invalid_num_lit, .info = missing_fraction_part}), "0xffaa._2139432");
}

TEST_SUITE_END();
#endif
