#ifndef AST_H
#define AST_H
#include "token.hpp"
#include "doctest.hpp"

#include <vector>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <fmt/core.h>

namespace ast {

typedef uint32_t token_index;
struct node;

struct tree {
  std::unique_ptr<node> root;
  const std::vector<token::type> tokens;
  const std::vector<module::span> token_locs;

  tree() = default;

  tree(std::unique_ptr<node> root,
       const std::vector<token::type> tokens,
       const std::vector<module::span> token_locs)
    : root(std::move(root)), tokens(tokens), token_locs(token_locs) {}
};

bool operator==(const tree& lhs, const tree& rhs);

doctest::String toString(const tree& tree);


enum class node_type : uint8_t {
  binop_expr,
  unop_expr,
  ident,
  int_lit,
  float_lit,
};

struct node {
  node_type type;
  token_index main_token;

  node(node_type type, token_index main_token) : type(type), main_token(main_token) {}
};

bool operator==(const node& lhs, const node& rhs);


enum class binop : uint8_t {
  add,
  sub,
  mul,
  div,
};

struct binop_expr : node {
  binop op;
  std::unique_ptr<node> lhs;
  std::unique_ptr<node> rhs;

  binop_expr(
    binop op,
    token_index main_token,
    std::unique_ptr<node> lhs = nullptr,
    std::unique_ptr<node> rhs = nullptr
  ) : node(node_type::binop_expr, main_token),
      op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
};

bool operator==(const binop_expr& lhs, const binop_expr& rhs);


enum class unop : uint8_t {
  neg,
  logical_not,
};

struct unop_expr : node {
  unop op;
  std::unique_ptr<node> operand;

  unop_expr(unop op, token_index main_token, std::unique_ptr<node> operand = nullptr)
    : node(node_type::unop_expr, main_token),
      op(op), operand(std::move(operand)) {}
};

bool operator==(const unop_expr& lhs, const unop_expr& rhs);


struct float_lit : node {
  float_lit(token_index token) : node(node_type::float_lit, token) {}
};


struct int_lit : node {
  int_lit(token_index token) : node(node_type::int_lit, token) {}
};

}; // End `ast` namespace.

#endif
