#include "lexer.hpp"
#include "error.hpp"
#include <fmt/core.h>

inline char Lexer::peek() const { return *current; }

// Checking to make sure that *current != EOF is not necessary because the file contents has an
// additional null byte appended to it.
inline char Lexer::peek_next() const { return current[1]; }

inline char Lexer::next() { return *current++; }

void Lexer::consume_whitespace() {
  for (;;) {
    switch (peek()) {
      case '/':
        // Handle line comments.
        if (peek_next() == '/') {
            current += 2;
            while (peek() != '\n' && peek() != '\0') { next(); };
        } else {
            return;
        }
        // Fallthrough enabled because current character must be '\n' or '\0'. It is useful to have
        // address of second EOF byte in `line_offsets` because it removes an edge case; a pointer
        // to the last character in any given line can be calculated with:
        // `line_offsets[<line-num>] - 2`
      case '\n':
        line_offsets.push_back(current + 1);
      case ' ':
      case '\t':
      case '\r':
      case '\f':
      case '\v':
        next();
        break;
      default:
        return;
    }
  }
}

inline Error Lexer::make_error(const std::string& msg) const {
  return Error(file, Span(start, current), msg);
}

inline Token Lexer::make_token(Token::Type kind) const {
  return Token(kind, Span(start, current));
}

Token Lexer::next_token() {
  consume_whitespace();
  start = current;
  switch (next()) {
    case '\0':
      return make_token(Token::Type::TOK_EOF);
    case 'a':
    case 'b':
    case 'c':
      return make_token(Token::Type::NUM);
    default:
      errors.push_back(make_error(fmt::format("unknown character: '{:c}'", current[-1])));
      return make_token(Token::Type::INVALID);
  }
}





