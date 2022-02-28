#ifndef LOCATION_H
#define LOCATION_H
#include <string>

// The address bounds [lo, hi) of a contiguous sequence of characters in a source file. Trying out
// using pointers instead of an index because the lexer works its way through the source via
// a pointer. Location is only needed when reporting errors and the pointers can be converted to an
// index at that point if it is needed.
struct Span {
  const char* lo;
  const char* hi;

  Span() = default;
  Span(const char* lo, const char* hi) : lo(lo), hi(hi) {}

  int len() const { return std::distance(lo, hi); }

  std::string_view contents() const { return std::string_view(lo, len()); }
};




#endif
