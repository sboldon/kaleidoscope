#ifndef MODULE_H
#define MODULE_H
#include <vector>
#include <string>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/color.h>

namespace fs = std::filesystem;

// Declared in error.hpp.
struct error_type;
struct error;

namespace module {

struct span;
struct file;

// TODO: If/when multiple source files are supported, all of them can have the same display context.
// Right now every file has a unique error handler, so some way to avoid creating a new display
// style for each file would be nice.
struct error_context {
  struct display_styles {
    fmt::text_style err_label;
    fmt::text_style msg;
    fmt::text_style arrow;
    fmt::text_style file_info;
    fmt::text_style caret;
  };

  const module::file& file;
  std::vector<error> errors;
  display_styles style;

  error_context(const module::file& file);

  bool has_error() const;
  void mark_error(error_type kind, const span& loc);
  void mark_error(error_type kind, const span& loc, uint32_t line_no, uint32_t num_lines);
  void display_errors() const;
  void display(const error& err) const;
};


// map from name -> srcfile ? however, is it just better to store ref to source file?
// Satisfies lexable_source concept.
struct file {
  fs::path name;
  std::string contents;
  std::vector<const char*> line_offsets; // The address of every character that begins a new line.
  error_context err_handler;

  file(fs::path name);

  const char* start();
  std::string_view line(uint32_t line_no, uint32_t num_lines = 1) const;
  bool has_error() const;
  void mark_error(error_type kind, const span& loc);
  void mark_error(error_type kind, const span& loc, uint32_t line_no, uint32_t num_lines);
  void display_errors() const;
};


// The address bounds [lo, hi) of a contiguous sequence of characters in a source file.
struct span {
  const char* lo;
  const char* hi;

  span() = default;
  span(const char* lo, const char* hi) : lo(lo), hi(hi) {}

  int len() const;
  std::string_view contents() const;
};


struct file_pos {
  uint32_t line_no;
  uint32_t col_no;
  uint32_t len;

  file_pos(const file& file, const span& loc);
  file_pos(uint32_t line_no, uint32_t col_no, uint32_t len)
    : line_no(line_no), col_no(col_no), len(len) {}
};

} // End module namespace.


template <> struct fmt::formatter<module::file_pos>: formatter<string_view> {
  template <typename Context>
  auto format(const module::file_pos& pos, Context& ctx) {
    auto end_col = pos.col_no + pos.len - 1;
    std::string repr =
      end_col > pos.col_no
        ? fmt::format("{}:{}-{}", pos.line_no, pos.col_no, end_col)
        : fmt::format("{}:{}", pos.line_no, pos.col_no);
    return fmt::formatter<string_view>::format(repr, ctx);
  }
};

#endif
