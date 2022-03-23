#ifndef PARSER_H
#define PARSER_H
#include "lexer.hpp"
#include "module.hpp"
#include "error.hpp"
#include <vector>

class parser {
  public:
    parser (module::file& file) : file(file), scanner(lexer(file)) {}
    void parse();

  private:
    module::file& file;
    lexer<module::file> scanner;
};

#endif
