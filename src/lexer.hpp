#ifndef LEXER_H
#define LEXER_H
#include "error.hpp"
#include "module.hpp"
#include "token.hpp"
#include <concepts>
#include <string>
#include <vector>

// A lexable source requires:
// - a vector to store the start of each line
// - a method that returns a pointer to the start of its contents
//   -- implicit requirement that contents must have two terminating null bytes
// - a method that indicates the presence of errors
// - a method to construct an error given its type and source location
template <typename T>
concept lexable_source = requires(T t) {
  requires std::same_as<decltype(T::line_offsets), std::vector<const char*>>;
  { t.start() } -> std::same_as<const char*>;
  { t.has_error() } -> std::same_as<bool>;
  requires requires(error_type kind, const module::span& loc, uint32_t line_no,
                    uint32_t num_lines) {
    { t.mark_error(kind, loc) } -> std::same_as<void>;
    // { t.mark_error(kind, loc, line_no, num_lines) } -> std::same_as<void>;
  };
};


template <lexable_source T> class lexer {
public:
  lexer(T &source)
    : source(source),
      line_offsets(source.line_offsets),
      start(source.start()),
      current(start) {}

  token next_token();

private:
  char peek() const;
  char peek_next() const;
  char next();
  token make_token(token::type kind) const;
  void mark_error(error_type kind);

  void consume_whitespace();
  void scan_ident_chars();
  token seen_keyword_char();

  void consume_invalid_num_lit();
  token::type scan_dec_digits();
  token::type seen_dec_point();
  token::type seen_exponent_char();
  token::type seen_dec_digit();
  token::type seen_bin_lit_prefix();
  token::type seen_oct_lit_prefix();
  token::type seen_hex_point();
  token::type seen_hex_lit_prefix();
  token::type seen_zero();

  // const source<module::file>& file;
  // const module::file& file;
  T& source;
  std::vector<const char*> &line_offsets;
  const char* start;
  const char* current;
  // std::vector<error>& errors;
};

#endif
