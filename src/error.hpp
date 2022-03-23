#ifndef ERROR_H
#define ERROR_H
#include "module.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <fmt/core.h>
#include <fmt/compile.h>
#include <fmt/color.h>


// Check if stderr is connected to a terminal.
bool stderr_has_color();


struct error_type {
  enum reason {
    unknown_char,
    invalid_num_lit
  };

  enum detail {
    none,
    // Numeric literals
    non_bin_digit,
    non_oct_digit,
    non_dec_digit,
    non_hex_digit,
    multiple_radix_points,
    missing_fraction_part,
    missing_exponent,
    unknown_radix_prefix
  };

  reason tag;
  union {
    detail info;
    char ch;
  };

  bool eq(const error_type& other);
};


// An error has a msg, a starting line number, a number of lines, and a file_pos. By default, the
// starting line number = the line number of the file_pos and the num_lines = 1. This way a range
// of lines can be displayed with the specific location of the error marked. It would also be good
// to add a way to suplement the error with additional notes that could be associated with a
// location if desired. Using these additional notes, different, but associated lines could be shown
// together to enhance the quality of the error messsage.
struct error {
  error_type kind;
  module::file_pos pos;
  uint32_t line_no = pos.line_no;
  uint32_t num_lines = 1;

  error(error_type kind, module::file_pos pos) : kind(kind), pos(pos) {}
  error(error_type kind, module::file_pos pos, uint32_t line_no, uint32_t num_lines)
    : kind(kind),
      pos(pos),
      line_no(line_no),
      num_lines(num_lines) {}

  static void simple_error(std::string_view msg);

private:
  static constexpr fmt::text_style err_style = fmt::emphasis::bold | fg(fmt::color::red);
  static constexpr fmt::text_style no_style;
};


template <> struct fmt::formatter<error_type::detail>: formatter<string_view> {
  using enum error_type::detail;

  template <typename Context>
  auto format(error_type::detail info, Context& ctx) {
    std::string repr;
    switch (info) {
      case non_bin_digit: repr = "expected binary digit"; break;
      case non_oct_digit: repr = "expected octal digit"; break;
      case non_dec_digit: repr = "expected digit"; break;
      case non_hex_digit: repr = "expected hexadecimal digit"; break;
      case multiple_radix_points: repr = "multiple radix points"; break;
      case missing_fraction_part: repr = "expected fraction part"; break;
      case missing_exponent: repr = "expected exponent"; break;
      case unknown_radix_prefix: repr = "unknown radix prefix"; break;
      default: repr = ""; break;
    }
    return fmt::formatter<string_view>::format(repr, ctx);
  }
};

template <> struct fmt::formatter<error_type>: formatter<string_view> {
  using enum error_type::reason;
  using enum error_type::detail;

  template <typename Context>
  auto format(const error_type& kind, Context& ctx) {
    std::string repr;
    switch (kind.tag) {
      case unknown_char:
        repr =
          (isprint(kind.ch)
             ? fmt::format(FMT_COMPILE("unknown character: '{:c}'"), kind.ch)
             : fmt::format(FMT_COMPILE("unknown byte: '{:#X}'"), kind.ch));
        break;
      case invalid_num_lit:
        repr =
          (kind.info == none
             ? "invalid numeric literal"
             : fmt::format(FMT_COMPILE("invalid numeric literal: {}"), kind.info));
        break;
    }
    return fmt::formatter<string_view>::format(repr, ctx);
  }
};

#endif
