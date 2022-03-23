#include "parser.hpp"
#include "token.hpp"
#include <fmt/core.h>

void parser::parse() {
  token cur;
  do {
    cur = scanner.next_token();
    fmt::print("{:<6} {}\n", module::file_pos(file, cur.loc), cur);
  } while (cur.kind != token::type::TOK_EOF);
}
