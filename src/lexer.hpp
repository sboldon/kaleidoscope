#ifndef LEXER_H
#define LEXER_H
#include "token.hpp"
#include "error.hpp"
#include <string>
#include <vector>

class Lexer {
  public:
    // Lexer(const std::string& src, std::vector<const char*>& line_offsets, std::vector<Error>& errors)
    //   : start(src.c_str()),
    //     line_offsets(line_offsets),
    //     errors(errors) {};
    Lexer(SrcFile& file, std::vector<Error>& errors) : file(file), start(file.contents.c_str()), current(start), errors(errors) {}
    Token next_token();
  private:
    inline char peek() const;
    inline char peek_next() const;
    inline char next();
    void consume_whitespace();
    inline Token make_token(Token::Type kind) const;
    inline Error make_error(const std::string& msg) const;

    SrcFile& file;
    std::vector<const char*>& line_offsets = file.line_offsets;
    const char* start;
    const char* current;
    std::vector<Error>& errors;
};

#endif
