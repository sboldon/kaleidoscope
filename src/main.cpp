#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.hpp"
#include "module.hpp"
#include "ast.hpp"
#include "ast_pretty_printer.hpp"
#include "parser.hpp"
#include "error.hpp"

#include <fmt/core.h>


// TODO: Command line argument parsing.
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
  parsing::parser parser(file);
  parser.parse();
  if (file.has_error()) {
    file.display_errors();
    return EXIT_FAILURE;
  }

  ast::pretty_printer<> pp(*file.abs_syntax, std::ostream_iterator<char>{std::cerr}, &file);
  pp.traverse_ast();
  return EXIT_SUCCESS;
}
