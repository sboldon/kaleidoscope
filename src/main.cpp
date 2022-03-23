#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "module.hpp"
#include "parser.hpp"
#include "error.hpp"

// TODO: Command line argument parsing
int main(int argc, char **argv) {
#if !defined(DOCTEST_CONFIG_DISABLE)
  doctest::Context ctx;
  ctx.setOption("abort-after", 5);              // Stop test execution after 5 failed assertions.
  ctx.setOption("order-by", "name");            // Sort the test cases by their name.
  ctx.applyCommandLine(argc, argv);
  int test_result = ctx.run();                  // Run testcases unless passed -nr option.
  if (ctx.shouldExit()) {                       // Run testcases and exit with -e option.
    return test_result;
  }
#endif
  module::file file(argv[1]);
  parser parser(file);
  parser.parse();
  if (file.has_error()) {
    file.display_errors();
    return EXIT_FAILURE;
  }
  // TODO: Semantic analysis
  return EXIT_SUCCESS;
}
