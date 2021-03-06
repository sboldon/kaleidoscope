#include "module.hpp"
#include "error.hpp"
#include "ast.hpp"    // Required for implicit `file` destructor.
#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <fmt/core.h>

namespace module {

//------------------------------------------------------------------------------------------------//
file::file(fs::path name) : name(name), err_handler(*this) {
  std::ifstream in_stream(name);
  if (!in_stream) {
    error::simple_error(fmt::format("unable to open '{}'", name.string()));
    exit(EXIT_FAILURE);
  }

  contents = std::string(std::istreambuf_iterator<char>{in_stream}, {});
  if (contents.size() > std::numeric_limits<uint32_t>::max()) {
    (error::simple_error
      (fmt::format("'{}' is too large: expected a file size less than 4096MB.", name.string())));
    exit(EXIT_FAILURE);
  }

  // Append additional null byte so that `lexer<T>::peek_next` will return a null byte on the last
  // byte. This avoids having to check for EOF every time that `lexer<T>::peek_next` is called.
  contents.push_back('\0');
  line_offsets.push_back(contents.c_str());
}

const char* file::start() const { return contents.data(); }

std::string_view file::line(uint32_t line_no, uint32_t num_lines) const {
  uint32_t idx = line_no - 1;
  const char* beg = line_offsets[idx];
  const char* end = line_offsets[idx + num_lines];
  return span(beg, end - 1).contents();
}

uint32_t file::estimate_num_tokens() const {
  // TODO: Refine estimate based on testing. This is a complete guess.
  return contents.size() / 10;
}

bool file::has_error() const { return err_handler.has_error(); }

void file::mark_error(error_type kind, const span& loc) {
  err_handler.mark_error(std::move(kind), loc);
}

void file::mark_error(error_type kind, const span& loc, uint32_t line_no, uint32_t num_lines) {
  err_handler.mark_error(std::move(kind), loc, line_no, num_lines);
}

void file::display_errors() const { err_handler.display_errors(); }

//------------------------------------------------------------------------------------------------//
int span::len() const {
  return std::distance(lo, hi);
}

std::string_view span::contents() const {
  return std::string_view(lo, len());
}

bool operator==(const span& lhs, const span& rhs) {
  return lhs.lo == rhs.lo && lhs.hi == rhs.hi;
}

//------------------------------------------------------------------------------------------------//
error_context::error_context(const module::file& file) : file(file) {
  if (stderr_has_color()) {
    style.err_label = fmt::emphasis::bold | fg(fmt::color::red);
    style.msg = fmt::emphasis::bold;
    style.arrow = fmt::emphasis::bold | fg(fmt::color::sky_blue);
    style.file_info = fmt::emphasis::italic;
    style.caret = style.err_label;
  }
}

bool error_context::has_error() const {
  return errors.size() > 0;
}

void error_context::mark_error(error_type kind, const span& loc) {
  errors.emplace_back(std::move(kind), file_pos(file, loc));
}

void error_context::mark_error
  (error_type kind,
   const span& loc,
   uint32_t line_no,
   uint32_t num_lines) {
  errors.emplace_back(std::move(kind), file_pos(file, loc), line_no, num_lines);
}

void error_context::display_errors() const {
  for (const auto& err : errors) {
    display(err);
    fmt::print(stderr, "\n");
  }
}

// TODO: Possibly remove explicit dependency on module::file via a concept. All that is required is a
// `line` method and a `name` member of type `fs::path`.
// TODO: Calculate column number in a Unicode friendly way. A column should be defined as either a code point or a
// grapheme cluster; currently not sure which is the best representation.
void error_context::display(const error& err) const {
  uint32_t line_after_err = err.line_no + err.num_lines;
  uint32_t num_line_digits = uint32_t(log10(line_after_err)) /* + 1 */;
  uint32_t line_no_display_width = (num_line_digits <= 3) ? 4 : num_line_digits + 1;

  // error: <msg>
  //    ==> <file-path>:<line-num>:<col-num>
  //     |
  (fmt::print
    (stderr,
     "{} {}\n   {} {}\n{:<{}} |\n",
     fmt::format(style.err_label, "error:"),
     fmt::format(style.msg, "{}", err.kind),
     fmt::format(style.arrow, "==>"),
     fmt::format(style.file_info,"{}:{}", file.name.string(), err.pos),
     "", line_no_display_width));

  // <line-no> | <source-code-line>
  //          ...
  //           |  <location-marker>
  // <line-no> | <source-code-line>
  //          ...
  for (auto i = err.line_no; i < line_after_err; i++) {
    (fmt::print
      (stderr,
       "{:>{}d} | {}\n",
       i, line_no_display_width,
       file.line(i)));
    if (i == err.pos.line_no) {
      (fmt::print
        (stderr,
         "{:<{}} | {}\n",
         "", line_no_display_width,
         fmt::format(style.caret, "{:>{}}", std::string(err.pos.len, '^'), err.pos.col_no)));
    }
  }
}

} // End `module` namespace.
