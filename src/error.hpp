#ifndef ERROR_H
#define ERROR_H
#include "srcfile.hpp"
#include "location.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <fmt/core.h>
#include <fmt/color.h>

// Double layer macro to be able to stringify expanded version of macros.
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define COMPILER_ERR(msg) \
  ("error: compiler bug in " __FILE__ " on line " TOSTRING(__LINE__) ": " msg)

// TODO: Set colorized output based on flag?
// TODO: Perhaps add a vector<Error> field for adding additional notes to the error? A use case for
// this would be displaying another code section that is tied to the error.
// The components of the current representation all seem needed to have a source-located error.
// However, there is no way to embed an error within a range of lines. Im thinking this can be done
// based off of AST as each node will be tagged with a span. This way, the definition of a struct
// could be shown on multiple lines with the specific line the error occurred being tagged with a
// message.
struct Error {
  const SrcFile& file;
  Span loc;
  std::string msg;

  Error(const SrcFile& file, const Span& loc, std::string msg);
  void display() const;

private:
  std::vector<const char*>::size_type line_start_idx;
  std::vector<const char*>::size_type line_end_idx;
  static constexpr fmt::text_style err_style = fmt::emphasis::bold | fg(fmt::color::red);
  static constexpr fmt::text_style no_style;

  void print_source_line(int line_start_idx, int line_no_display_width) const;
  static bool stderr_has_color();
};




#endif
