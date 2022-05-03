#include "parser.hpp"
#include "module.hpp"
#include "lexer.hpp"
#include "token.hpp"
#include "error.hpp"
#include "doctest.hpp"

#include <functional>
#include <optional>
#include <tuple>
#include <fmt/core.h>

namespace parsing {

template <parseable T> void parser<T>::parse() {
  tokenize();
  std::unique_ptr<ast::node> root = expression();
  // Take ownership of `tokens` and `token_locs`.
  source.abs_syntax = std::make_unique<ast::tree>(std::move(root), this->tokens, this->token_locs);
}

template <parseable T> void parser<T>::tokenize() {
  lexer scanner(source);
  uint32_t estimated_token_count = source.estimate_num_tokens();
  std::vector<token::type> tokens;
  std::vector<module::span> token_locs;
  tokens.reserve(estimated_token_count);
  token_locs.reserve(estimated_token_count);

  token cur;
  do {
    cur = scanner.next_token();
    fmt::print("{:<8} {}\n", module::file_pos(source, cur.loc), cur);
    tokens.push_back(cur.kind);
    token_locs.push_back(cur.loc);
  } while (cur.kind != token::type::eof);

  this->tokens = std::move(tokens);
  this->token_locs = std::move(token_locs);
}

template <parseable T> void parser<T>::expect(token::type expected_tok) {
  if (tokens[idx] == expected_tok) {
    idx += 1;
  } else {
    (source.mark_error
      ({.tag = error_type::reason::expected_token, .token = expected_tok},
       token_locs[idx]));
  }
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::expression() {
  return parse_precedence(precedence::term);
}

template <parseable T>
std::unique_ptr<ast::node> parser<T>::parse_precedence(precedence min_prec) {
  const auto& prefix_rule = rules[tokens[idx]];
  // fmt::print("token of prefix rule: {}\n", tokens[idx]);
  if (prefix_rule.prefix_action) {
    std::unique_ptr<ast::node> prefix_node = std::invoke(prefix_rule.prefix_action, this);
    while (static_cast<uint8_t>(rules[tokens[idx]].prec) >= static_cast<uint8_t>(min_prec)) {
      const auto& infix_rule = rules[tokens[idx]];
      std::unique_ptr<ast::node> infix_node = std::invoke(infix_rule.infix_action, this);
      auto binop = (ast::binop_expr*)infix_node.release();
      binop->lhs = std::move(prefix_node);
      prefix_node = std::unique_ptr<ast::binop_expr>(binop);
    }
    return prefix_node;
  }
  return nullptr;
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::binary() {
  ast::binop oper;
  token::type tok = tokens[idx];
  switch (tok) {
    case token::type::plus: oper = ast::binop::add; break;
    case token::type::dash: oper = ast::binop::sub; break;
    case token::type::star: oper = ast::binop::mul; break;
    case token::type::fwd_slash: oper = ast::binop::div; break;
    default: break; // unreachable
  }
  const auto& op_rule = rules[tok];
  auto binop = std::make_unique<ast::binop_expr>(oper, idx++);
  binop->rhs =
    (parse_precedence
      (static_cast<precedence>
        (static_cast<uint8_t>(op_rule.prec) + 1)));
  return binop;
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::unary() {
  ast::unop oper;
  token::type tok = tokens[idx];
  switch (tok) {
    case token::type::dash: oper = ast::unop::neg; break;
    case token::type::bang: oper = ast::unop::logical_not; break;
    default: break; // unreachable
  }
  return std::make_unique<ast::unop_expr>(oper, idx++, parse_precedence(precedence::unary));
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::grouping() {
  idx += 1; // Consume left paren.
  std::unique_ptr<ast::node> expr = expression();
  expect(token::type::right_paren);
  return expr;
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::var() {
  return std::make_unique<ast::node>(ast::node_type::ident, idx++);
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::float_literal() {
  return std::make_unique<ast::float_lit>(idx++);
}

template <parseable T> std::unique_ptr<ast::node> parser<T>::int_literal() {
  // fmt::print("int_literal @ idx: {}\n", idx);
  return std::make_unique<ast::int_lit>(idx++);
}


// Declaring the specific `parseable` types used with `parser<T>` allows the implementation of
// `parser<T>` to be separate from its declaration.
template class parser<module::file>;


//------------------------------------------------------------------------------------------------//
#if !defined(DOCTEST_CONFIG_DISABLE)
uint32_t parser_test_source::estimate_num_tokens() const {
  return 32;
}

static void test(const char* source_chars, std::unique_ptr<ast::node> expected) {
  CAPTURE(source_chars);
  parser_test_source source(source_chars);
  parser p(source);
  p.parse();

  // The tokens generated during parsing are able to be copied into `expected_tree` because parser
  // testcases make the assumption that the lexer is working correctly. Additionally, because the
  // representation used for ASTs does not directly contain any tokens, the result of an equality
  // test is unaffected.
  ast::tree expected_tree(std::move(expected),
                          p.source.abs_syntax->tokens,
                          p.source.abs_syntax->token_locs);
  CHECK(*p.source.abs_syntax == expected_tree);
}

// Reduce the boilerplate that is required to make an AST literal.
template <typename T, typename ...Args>
inline auto mk(Args&&... args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

TEST_SUITE_BEGIN("parsing");

TEST_CASE("expressions") {
  using namespace ast;
  test("1", mk<int_lit>(0));
  test("1.0", mk<float_lit>(0));

  test("-1", mk<unop_expr>(unop::neg, 0, mk<int_lit>(1)));
  test("!1", mk<unop_expr>(unop::logical_not, 0, mk<int_lit>(1)));

  test("--1",
    mk<unop_expr>(unop::neg, 0,
      mk<unop_expr>(unop::neg, 1, mk<int_lit>(2))));

  test("1 + 2", mk<binop_expr>(binop::add, 1, mk<int_lit>(0), mk<int_lit>(2)));
  test("1 - 2", mk<binop_expr>(binop::sub, 1, mk<int_lit>(0), mk<int_lit>(2)));
  test("1 * 2", mk<binop_expr>(binop::mul, 1, mk<int_lit>(0), mk<int_lit>(2)));
  test("1 / 2", mk<binop_expr>(binop::div, 1, mk<int_lit>(0), mk<int_lit>(2)));

  test("1 + 2 - 3",
    mk<binop_expr>(binop::sub, 3,
      mk<binop_expr>(binop::add, 1, mk<int_lit>(0), mk<int_lit>(2)),
      mk<int_lit>(4)));
  test("1 + (2 - 3)",
    mk<binop_expr>(binop::add, 1,
      mk<int_lit>(0),
      mk<binop_expr>(binop::sub, 4, mk<int_lit>(3), mk<int_lit>(5))));
  test("1 + 2 * 3",
    mk<binop_expr>(binop::add, 1,
      mk<int_lit>(0),
      mk<binop_expr>(binop::mul, 3, mk<int_lit>(2), mk<int_lit>(4))));

  test("1 - -2",
    mk<binop_expr>(binop::sub, 1,
      mk<int_lit>(0),
      mk<unop_expr>(unop::neg, 2, mk<int_lit>(3))));
}

TEST_SUITE_END();
#endif

} // End `parsing` namespace
