#include "error.hpp"
// TODO: Put this is place where other platform specific code is needed?
#if defined(_WIN32)
#define OS_WINDOWS
#include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#define OS_POSIX
#include <unistd.h>
#endif

// Check if stderr is connected to a terminal.
bool Error::stderr_has_color() {
#if defined(OS_WINDOWS)
  return _isatty(_fileno(stderr)) != 0; // TODO: This is never true on Cygwin.
#elif defined(OS_POSIX)
  return isatty(STDERR_FILENO) != 0;
#endif
  return false;
}


Error::Error(const SrcFile& file, const Span& loc, std::string msg) : file(file), loc(loc), msg(msg) {
  // for (const auto ptr : file.line_offsets) {
  //   fmt::print("{:d}\n", std::distance(file.contents.c_str(), ptr));
  // }
  // Obtain the index of the line that the span begins on and the index of the line after the span
  // ends.
  auto it = std::lower_bound(file.line_offsets.begin(), file.line_offsets.end(), loc.lo);
  if (it == file.line_offsets.end()) {
    // The span is on the line that is currently being processed.
    line_start_idx = file.line_offsets.size() - 1;
    // std::cerr << COMPILER_ERR("failed to obtain the line number of a Span") << "\n";
    // exit(EXIT_FAILURE);
  } else {
    line_start_idx = std::distance(file.line_offsets.begin(), it);
  }
  // fmt::print("line_start_idx: {:d}\n", line_start_idx);
  line_end_idx = line_start_idx;
  for (auto len = file.line_offsets.size();
       file.line_offsets[line_end_idx] <= loc.hi && line_end_idx < len;
       line_end_idx++);
}

void Error::display() const {
  const char* line_start_pos = file.line_offsets[line_start_idx];
  // const char* next_line_pos = file.line_offsets[line_end_idx];
  int col_no = std::distance(line_start_pos, loc.lo) + 1;
  int line_no = line_start_idx + 1;
  int num_line_digits = int(log10(line_no)) + 1;
  int line_no_display_width = (num_line_digits <= 3) ? 4 : num_line_digits + 1;

  if (stderr_has_color()) {
    // error: <msg>
    //    ==> <file-path>:<line-num>:<col-num>
    //     |
    fmt::print(stderr, "{} {}\n   {} {}\n{:<{}} |\n",
        fmt::format(err_style, "error:"),
        fmt::format(fmt::emphasis::bold, msg),
        fmt::format(fmt::emphasis::bold | fg(fmt::color::sky_blue), "==>"),
        fmt::format(fmt::emphasis::italic,"{}:{:d}:{:d}", file.name.string(), line_no, col_no),
        "", line_no_display_width
    );
    // <line-no> | <source-code-line>
    print_source_line(line_start_idx, line_no_display_width);
    //           |  <location-marker>
    fmt::print(stderr, "{:<{}} | {}\n",
        "", line_no_display_width,
        fmt::format(err_style | fmt::emphasis::blink, "{:>{}}",
          std::string(loc.len(), '^'), col_no)
    );
  } else {
    fmt::print(stderr, "{} {}\n   {} {}\n{:<{}} |\n",
        fmt::format("error:"),
        fmt::format(msg),
        fmt::format("==>"),
        fmt::format("{}:{:d}:{:d}", file.name.string(), line_no, col_no),
        "", line_no_display_width
    );
    print_source_line(line_start_idx, line_no_display_width);
    fmt::print(stderr, "{:<{}} | {}\n",
        "", line_no_display_width,
        fmt::format("{:>{}}", std::string(loc.len(), '^'), col_no)
    );
  }
}


// TODO: Perhaps truncate if line length if > 80 or 100? Only issue is have to make sure
// that the part of line inside of span is shown.
// <line-no> | <source-code-line>
void Error::print_source_line(int line_start_idx, int line_no_display_width) const {
  const char* line_pos = file.line_offsets[line_start_idx];
  const char* next_line_pos = file.line_offsets[line_start_idx + 1];
  fmt::print(stderr, "{:>{}d} | {}\n",
      line_start_idx + 1, line_no_display_width,
      Span(line_pos, next_line_pos - 1).contents());
}

