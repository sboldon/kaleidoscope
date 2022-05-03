#ifndef PARSER_H
#define PARSER_H
#include "lexer.hpp"
#include "ast.hpp"
#include "module.hpp"
#include "error.hpp"

#include <vector>
#include <array>
#include <fmt/core.h>

namespace parsing {

template <typename T>
concept parseable = lexable<T> && requires(T t, const T& const_t) {
  requires std::same_as<decltype(T::abs_syntax), std::unique_ptr<ast::tree>>;
  { const_t.estimate_num_tokens() } -> std::same_as<uint32_t>;
};

template <parseable T> class parser;
template <parseable T> using parser_rule_fn = std::unique_ptr<ast::node> (parser<T>::*)();

enum class precedence {
  none,
  term,   // + -
  factor, // * /
  unary,  // -
};


// The parsing rule associated with each token when it begins an expression or acts as a
// binary operator.
template <parseable T> struct rule {
  parser_rule_fn<T> prefix_action;
  parser_rule_fn<T> infix_action;
  precedence prec;

  constexpr rule() : prefix_action(nullptr), infix_action(nullptr), prec(precedence::none) {}
  constexpr rule(auto prefix_action, auto infix_action, precedence prec)
    : prefix_action(prefix_action),
      infix_action(infix_action),
      prec(prec) {}
};

template <parseable T> class parser {
  public:
    T& source;

    parser (T& source) : source(source), idx(0) {}

    void parse();
    std::unique_ptr<ast::node> expression();

  private:
    ast::token_index idx;
    std::vector<token::type> tokens;
    std::vector<module::span> token_locs;

    void tokenize();
    void expect(token::type);
    std::unique_ptr<ast::node> parse_precedence(precedence min_prec);
    std::unique_ptr<ast::node> binary();
    std::unique_ptr<ast::node> unary();
    std::unique_ptr<ast::node> grouping();
    std::unique_ptr<ast::node> var();
    std::unique_ptr<ast::node> int_literal();
    std::unique_ptr<ast::node> float_literal();

    // Essentially a C99 designated initializer of the form `{ [<enum-tag>] = <val>, ... }`.
    static constexpr auto rules = []{
      using enum token::type;
      std::array<rule<T>, token::type::num_tokens> rules{};

      rules[ident] = rule<T>(&parser::var, nullptr, precedence::none);
      rules[int_literal] = rule<T>(&parser::int_literal, nullptr, precedence::none);
      rules[float_literal] = rule<T>(&parser::float_literal, nullptr, precedence::none);

      rules[left_paren] = rule<T>(&parser::grouping, nullptr, precedence::none);

      rules[plus] = rule<T>(nullptr, &parser::binary, precedence::term);
      rules[dash] = rule<T>(&parser::unary, &parser::binary, precedence::term);

      rules[star] = rule<T>(nullptr, &parser::binary, precedence::factor);
      rules[fwd_slash] = rule<T>(nullptr, &parser::binary, precedence::factor);

      rules[bang] = rule<T>(&parser::unary, nullptr, precedence::unary);

      return rules;
    }();
};

#if !defined(DOCTEST_CONFIG_DISABLE)
// Satisfies `parseable` concept.
struct parser_test_source : lexer_test_source {
  std::unique_ptr<ast::tree> abs_syntax;
  using lexer_test_source::lexer_test_source;
  uint32_t estimate_num_tokens() const;
};
static_assert(parseable<parser_test_source>);
#endif

} // End `parsing` namespace.

#endif
