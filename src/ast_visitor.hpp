#ifndef AST_VISITOR_H
#define AST_VISITOR_H

#include "ast.hpp"

namespace ast {

// A generic AST visitor implemented via CRTP.
template <typename Derived> struct visitor {
  const tree& abs_syntax;

  visitor(const tree& abs_syntax);

  void traverse_ast();
  void visit(node& node);
  void visit(binop_expr& binop_node);
  void visit(unop_expr& unop_node);
  void visit(int_lit& int_node);
  void visit(float_lit& float_node);
private:
  Derived& derived();
};

template <typename Derived>
visitor<Derived>::visitor(const tree& abs_syntax) : abs_syntax(abs_syntax) {
  // fmt::print("`visitor::visitor` constructor called\n");
}

template <typename Derived> Derived& visitor<Derived>::derived() {
  return *static_cast<Derived*>(this);
}

template <typename Derived> void visitor<Derived>::traverse_ast() {
  // fmt::print("`visitor::traverse_ast` called\n");
  if (abs_syntax.root) {
    visit(*abs_syntax.root);
  }
}

// Invoke specialized behavior based on the type of an AST node. This is analogous to the double
// dispatch that is achieved via the `accept` method of each derived node in a class hierarchy with
// virtual methods.
// For all `visit(<node-type>)` methods that are unimplemented in the derived
// visitor, `visitor<T>::visit(<node-type>)` is called.
template <typename Derived> void visitor<Derived>::visit(node& node) {
  // fmt::print(stderr, "`visitor<T>::visit(node&)`: start of function\n");
  auto& self = derived();
  switch (node.type) {
    case node_type::binop_expr: {
      auto& binop_node = static_cast<binop_expr&>(node);
      if constexpr(requires(binop_expr& node) { self.visit(node); }) {
        // fmt::print(stderr, "`visitor<T>::visit(node&)`: calling derived visit(binop_expr&)\n");
        self.visit(binop_node);
      } else {
        visit(binop_node);
      }
      break;
    }
    case node_type::unop_expr: {
      auto& unop_node = static_cast<unop_expr&>(node);
      if constexpr(requires(unop_expr& node) { self.visit(node); }) {
        // fmt::print(stderr, "`visitor<T>::visit(node&)`: calling derived visit(unop_expr&)\n");
        self.visit(unop_node);
      } else {
        visit(unop_node);
      }
      break;
    }
    case node_type::ident: {
      // TODO
      break;
    }
    case node_type::int_lit: {
      auto& int_node = static_cast<int_lit&>(node);
      if constexpr(requires(int_lit& node) { self.visit(node); }) {
        // fmt::print(stderr,"`visitor<T>::visit(node&)`: calling derived visit(int_lit&)\n");
        self.visit(int_node);
      } else {
        visit(int_node);
      }
      break;
    }
    case node_type::float_lit: {
      auto& float_node = static_cast<float_lit&>(node);
      if constexpr(requires(float_lit& node) { self.visit(node); }) {
        // fmt::print(stderr,"`visitor<T>::visit(node&)`: calling derived visit(float_lit&)\n");
        self.visit(float_node);
      } else {
        visit(float_node);
      }
      break;
    }
  }
}

template <typename Derived> void visitor<Derived>::visit(binop_expr& binop_node) {}
template <typename Derived> void visitor<Derived>::visit(unop_expr& unop_node) {}
template <typename Derived> void visitor<Derived>::visit(int_lit& int_node) {}
template <typename Derived> void visitor<Derived>::visit(float_lit& float_node) {}

} // End `ast` namespace.

#endif
