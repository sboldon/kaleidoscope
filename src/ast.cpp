#include "ast.hpp"
#include "ast_pretty_printer.hpp"

namespace ast {

bool operator==(const tree& lhs, const tree& rhs) {
  return ((!lhs.root && !rhs.root) || (lhs.root && rhs.root && *lhs.root == *rhs.root)) &&
         lhs.tokens == rhs.tokens &&
         lhs.token_locs == rhs.token_locs;
}

doctest::String toString(const tree& tree) {
  std::string str;
  pretty_printer pp(tree, std::back_inserter(str));
  pp.traverse_ast();
  str.insert(0, "\n");
  // This is not a dangling pointer because `doctest::String` constructor makes a copy of the
  // string's contents.
  return str.c_str();
}

bool operator==(const node& lhs, const node& rhs) {
  if (lhs.type != rhs.type || lhs.main_token != rhs.main_token) {
    return false;
  }
  switch (lhs.type) {
    case node_type::binop_expr:
      return static_cast<const binop_expr&>(lhs) == static_cast<const binop_expr&>(rhs);
    case node_type::unop_expr:
      return static_cast<const unop_expr&>(lhs) == static_cast<const unop_expr&>(rhs);
    case node_type::ident:
      // TODO
      return false;
    case node_type::int_lit:
    case node_type::float_lit:
      return true;
  }
}

bool operator==(const binop_expr& lhs, const binop_expr& rhs) {
  return lhs.op == rhs.op &&
         ((!lhs.lhs && !rhs.lhs) || (lhs.lhs && rhs.lhs && *lhs.lhs == *rhs.lhs)) &&
         ((!lhs.rhs && !rhs.rhs) || (lhs.rhs && rhs.rhs && *lhs.rhs == *rhs.rhs));
}

bool operator==(const unop_expr& lhs, const unop_expr& rhs) {
  return lhs.op == rhs.op &&
         ((!lhs.operand && !rhs.operand) ||
          (lhs.operand && rhs.operand && *lhs.operand == *rhs.operand));
}


}; // End `ast` namespace.
