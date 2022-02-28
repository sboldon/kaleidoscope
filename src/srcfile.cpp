#include "srcfile.hpp"
#include <iostream>
#include <fstream>
#include <iterator>
#include <filesystem>
#include <fmt/core.h>

namespace fs = std::filesystem;

SrcFile::SrcFile(fs::path name) : name(name) {
  std::ifstream in_stream(name);
  if (!in_stream) {
    // TODO: Possibly incorporate this error with other error handling?
    fmt::print(stderr, "error: unable to open '{}'\n", name.string());
    exit(EXIT_FAILURE);
  }
  contents = std::string(std::istreambuf_iterator<char>{in_stream}, {});
  // Append additional null byte so that `Lexer::peek_next` will return a null byte on the last byte. This
  // avoids having to check for EOF every time that `Lexer::peek_next` is called.
  contents.push_back('\0');
  line_offsets.push_back(contents.c_str());
}

