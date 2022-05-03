#ifndef AST_PRETTY_PRINT_H
#define AST_PRETTY_PRINT_H

#include "ast.hpp"
#include "ast_visitor.hpp"

#include <deque>

namespace ast {

template <typename OutputIter=std::ostream_iterator<char>>
struct pretty_printer : visitor<pretty_printer<OutputIter>> {
  struct separator_chars {
    const char* branch = "│  ";
    const char* leaf = "├──";
    const char* last_leaf = "└──";
  };

  OutputIter out = std::ostream_iterator<char>(std::cout);
  const module::file *const source_file = nullptr;
  std::deque<const char*> branches;
  separator_chars separators;

  pretty_printer(const tree& abs_syntax) : visitor<pretty_printer>(abs_syntax) {}

  pretty_printer(const tree& abs_syntax, OutputIter out)
    : visitor<pretty_printer>(abs_syntax), out(out) {}

  pretty_printer(const tree& abs_syntax, const module::file *const source_file)
    : visitor<pretty_printer>(abs_syntax), source_file(source_file) {}

  pretty_printer(const tree& abs_syntax, OutputIter out, const module::file *const source_file)
    : visitor<pretty_printer>(abs_syntax), out(out), source_file(source_file) {}

  void visit(binop_expr& binop_node);
  void visit(unop_expr& unop_node);
  void visit(float_lit& float_node);
  void visit(int_lit& int_node);

  void print_branches() const;
  void print_loc(const module::span& loc) const;
};


// template <typename T> void pretty_printer<T>::print_indent() const {
//     fmt::format_to(out, "{:{}}", "", indent);
// }

template <typename T> void pretty_printer<T>::visit(binop_expr& binop_node) {
  // const module::span& loc =
  //   static_cast<T*>(this)->abs_syntax.token_locs[binop_node.main_token];
  const module::span& loc = this->abs_syntax.token_locs[binop_node.main_token];
  // print_indent();
  fmt::format_to(out, "BinaryOperator `{:s}`", loc.contents());
  print_loc(loc);
  // indent += indent_size;
  if (!branches.empty()) {
    branches.back() = separators.branch;
  }
  if (binop_node.lhs) {
    // fmt::print(stderr,"`pretty_printer<T>::visit(binop_expr&)`: calling visit on lhs\n");
    branches.push_back(separators.leaf);
    print_branches();
    visitor<pretty_printer>::visit(*binop_node.lhs);
  }
  if (binop_node.rhs) {
    // fmt::print(stderr,"`pretty_printer<T>::visit(binop_expr&)`: calling visit on rhs\n");
    branches.back() = separators.last_leaf;
    print_branches();
    visitor<pretty_printer>::visit(*binop_node.rhs);
  }
  // indent -= indent_size;
  // branches.front() = separators.leaf;
  if (binop_node.lhs || binop_node.rhs) {
    branches.pop_back();
  }
}

template <typename T> void pretty_printer<T>::visit(unop_expr& unop_node) {
  const module::span& loc = this->abs_syntax.token_locs[unop_node.main_token];
  // print_indent();
  fmt::format_to(out, "UnaryOperator `{:s}`", loc.contents());
  print_loc(loc);
  // indent += indent_size;
  if (!branches.empty()) {
    branches.back() = separators.branch;
  }
  if (unop_node.operand) {
    branches.push_back(separators.last_leaf);
    print_branches();
    visitor<pretty_printer>::visit(*unop_node.operand);
    branches.pop_back();
  }
  // indent -= indent_size;
}

template <typename T> void pretty_printer<T>::visit(float_lit& float_node) {
  const module::span& loc = this->abs_syntax.token_locs[float_node.main_token];
  fmt::format_to(out, "FloatLiteral `{:s}`", loc.contents());
  print_loc(loc);
}

template <typename T> void pretty_printer<T>::visit(int_lit& int_node) {
  // fmt::print("`pretty_printer<T>::visit(int_lit&)`: start of function\n");
  const module::span& loc = this->abs_syntax.token_locs[int_node.main_token];
  fmt::format_to(out, "IntLiteral `{:s}`", loc.contents());
  print_loc(loc);
}

template <typename T> void pretty_printer<T>::print_branches() const {
  for (auto branch_type : branches) {
    fmt::format_to(out, "{} ", branch_type);
  }
}

template <typename T> void pretty_printer<T>::print_loc(const module::span& loc) const {
  if (source_file) {
    fmt::format_to(out, " {}\n", module::file_pos(*source_file, loc));
  } else {
    fmt::format_to(out, "\n");
  }
}

} // End `ast` namespace.

#endif
