#include "error.hpp"
#include "module.hpp"

// TODO: Put this is place where other platform specific code is needed?
#if defined(_WIN32)
#define OS_WINDOWS
#include <Windows.h>
#elif defined(__unix__) || defined(__APPLE__)
#define OS_POSIX
#include <unistd.h>
#endif


bool stderr_has_color() {
#if defined(OS_WINDOWS)
  return _isatty(_fileno(stderr)) != 0; // TODO: This is never true on Cygwin.
#elif defined(OS_POSIX)
  return isatty(STDERR_FILENO) != 0;
#endif
  return false;
}


bool error_type::eq(const error_type& other) {
  if (other.tag != tag) {
    return false;
  }
  switch (other.tag) {
  case unknown_char: return other.ch == ch;
  case invalid_num_lit: return other.info == info;
  default: return false;
  }
}


void error::simple_error(std::string_view msg) {
  fmt::text_style style = stderr_has_color() ? err_style : no_style;
  fmt::print(stderr, "{} {}\n", fmt::format(style, "error:"), msg);
}
