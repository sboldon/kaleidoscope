#ifndef PARSER_H
#define PARSER_H
#include "lexer.hpp"
#include "srcfile.hpp"
#include "error.hpp"
#include <vector>

class Parser {
  public:
    // Parser(SrcFile& file) : file(file), scanner(Lexer(file.contents, file.line_offsets, errors)) {}
    Parser (SrcFile& file) : file(file), scanner(Lexer(file, errors)) {}
    void parse();
    bool is_error() const;


    std::vector<Error> errors;

  private:
    void temp_method() { std::cout << file.contents[0]; }
    SrcFile& file;
    Lexer scanner;
};

#endif
