#include "srcfile.hpp"
#include "parser.hpp"
#include "error.hpp"
#include <fmt/core.h>

// TODO: Command line argument parsing
int main(int argc, char *const*const argv) {
  std::cout << argv[1] << "\n";
  SrcFile file(argv[1]);
  Parser parser(file);
  parser.parse();
  if (parser.is_error()) {
    for (const auto& err : parser.errors) { err.display(); }
    return EXIT_FAILURE;
  }
  // TODO: Semantic analysis
  return EXIT_SUCCESS;
}
