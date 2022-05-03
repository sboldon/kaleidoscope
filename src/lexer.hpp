#ifndef LEXER_H
#define LEXER_H
#include "error.hpp"
#include "module.hpp"
#include "token.hpp"
#include <concepts>
#include <string>
#include <vector>

#include <optional>

template <typename T>
concept reports_errors = requires(T t, error_type kind, const module::span& loc, uint32_t line_no, uint32_t num_lines) {
  { t.mark_error(kind, loc) } -> std::same_as<void>;
  // { t.mark_error(kind, loc, line_no, num_lines) } -> std::same_as<void>;
};

// A lexable source requires:
// - an `error` reporting method
// - a vector storing pointers to the start of each line in the source
// - a method that returns a pointer to the start of its contents
//   -- implicit requirement that contents must have two terminating null bytes
// - a method that indicates the presence of errors
template <typename T>
concept lexable = reports_errors<T> && requires(T t, const T& const_t) {
  requires std::same_as<decltype(T::line_offsets), std::vector<const char*>>;
  { const_t.start() } -> std::same_as<const char*>;
  { const_t.has_error() } -> std::same_as<bool>;
};


template <lexable T> class lexer {
public:
  lexer(T &source)
    : source(source),
      line_offsets(source.line_offsets),
      start(source.start()),
      current(start) {}

  token next_token();

private:
  T& source;
  std::vector<const char*> &line_offsets;
  const char* start;
  const char* current;

  char peek() const;
  char peek_next() const;
  char next();
  token make_token(token::type kind) const;
  void mark_error(error_type kind);

  void consume_whitespace();
  void scan_ident_chars();
  token seen_keyword_char();

  void consume_invalid_num_lit(error_type::detail);
  token::type scan_dec_digits();
  token::type seen_dec_point();
  token::type seen_exponent_char();
  token::type seen_dec_digit();
  token::type seen_bin_lit_prefix();
  token::type seen_oct_lit_prefix();
  token::type seen_hex_point();
  token::type seen_hex_lit_prefix();
  token::type seen_zero();
};


#if !defined(DOCTEST_CONFIG_DISABLE)
// Satisfies `lexable` concept and tracks errors for testing the lexer's error handling.
struct lexer_test_source {
  std::string contents;
  std::vector<const char*> line_offsets;
  std::optional<error_type> err_reason;

  lexer_test_source(const char* buf);

  const char* start() const;
  bool has_error() const;
  void mark_error(error_type kind, const module::span& loc);
};
static_assert(lexable<lexer_test_source>);
#endif

#endif
