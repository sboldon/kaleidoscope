#ifndef MODULE_H
#define MODULE_H
#include <vector>
#include <string>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ranges.h>

namespace fs = std::filesystem;

// Declared in error.hpp.
struct error_type;
struct error;

// Declared in ast.hpp.
namespace ast {
  struct tree;
}

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


// Satisfies `parseable` concept.
struct file {
  fs::path name;
  std::vector<const char*> line_offsets; // The address of every character that begins a new line.
  std::unique_ptr<ast::tree> abs_syntax;

  file(fs::path name);

  const char* start() const;
  std::string_view line(uint32_t line_no, uint32_t num_lines = 1) const;
  uint32_t estimate_num_tokens() const;

  bool has_error() const;
  void mark_error(error_type kind, const span& loc);
  void mark_error(error_type kind, const span& loc, uint32_t line_no, uint32_t num_lines);
  void display_errors() const;

private:
  std::string contents;
  error_context err_handler;
};


// TODO: Replace `span` and `file_pos` with:
// struct span {
//   const char* start;
//   uint32_t line_no;
//   uint16_t col_no;
//   uint16_t len;
// }
// It is the same size as current `span` yet contains the information of both. Alternatively,
// uint32_t indices could be used for lo & hi resultig in 8 byte size instead of 16.

// The address bounds [lo, hi) of a contiguous sequence of characters in a source file.
struct span {
  const char* lo;
  const char* hi;

  span() = default;
  span(const char* lo, const char* hi) : lo(lo), hi(hi) {}

  int len() const;
  std::string_view contents() const;
};

bool operator==(const span& lhs, const span& rhs);


struct file_pos {
  uint32_t line_no;
  uint16_t col_no;
  uint16_t len;

  // Requires a type that satisfies `lexable` concept.
  file_pos(const auto& source, const span& loc);
  file_pos(uint32_t line_no, uint32_t col_no, uint32_t len)
    : line_no(line_no), col_no(col_no), len(len) {}
};

inline file_pos::file_pos(const auto& source, const span& loc) {
  uint32_t line_start_idx;
  // fmt::print("line_offsets: {}\n", file.line_offsets);
  auto iter = std::lower_bound(source.line_offsets.begin(), source.line_offsets.end(), loc.lo);
  if (iter != source.line_offsets.begin()) {
    // `std::lower_bound` returns an iterator pointing to the start of the first line that is not
    // less than `loc.lo`. We want the start of the first line that is not greater than `loc.lo`.
    iter -= 1;
  }
  // if (iter == file.line_offsets.end()) {
  //   // The span is on the line that is currently being processed (or the last line of the file).
  //   fmt::print("here?\n");
  //   line_start_idx = file.line_offsets.size() - 1;
  // } else {
  //   line_start_idx = std::distance(file.line_offsets.begin(), iter);
  // }
  line_start_idx = std::distance(source.line_offsets.begin(), iter);
  // fmt::print("`line_start_idx`: {}\n", line_start_idx);
  const char* line_start = source.line_offsets[line_start_idx];
  // fmt::print("`line_start`:{} `loc.lo`:{}\n", fmt::ptr(line_start), fmt::ptr(loc.lo));
  line_no = line_start_idx + 1;
  col_no = std::distance(line_start, loc.lo) + 1;
  len = loc.len();
}

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
