#include "parser.hpp"
#include "token.hpp"

bool Parser::is_error() const {
  return errors.size() > 0;
}

void Parser::parse() {
  Token cur;
  do {
    cur = scanner.next_token();
  } while (cur.kind != Token::Type::TOK_EOF);
}
