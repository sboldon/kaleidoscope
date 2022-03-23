// Macros that are used to mimic GCC switch case range compiler extension.
#ifndef LEXER_PATTERNS_H
#define LEXER_PATTERNS_H

#define OCTAL \
  case '0':   \
  case '1':   \
  case '2':   \
  case '3':   \
  case '4':   \
  case '5':   \
  case '6':   \
  case '7':

#define DECIMAL \
  OCTAL         \
  case '8':     \
  case '9':

#define LOWER_ALPHA_HEX \
  case 'a':             \
  case 'b':             \
  case 'c':             \
  case 'd':             \
  case 'e':             \
  case 'f':             \

#define UPPER_ALPHA_HEX \
  case 'A':             \
  case 'B':             \
  case 'C':             \
  case 'D':             \
  case 'E':             \
  case 'F':

#define HEX       \
  DECIMAL         \
  LOWER_ALPHA_HEX \
  UPPER_ALPHA_HEX

// Removal of hex digits and hex floating point scientific notation character 'p'.
#define LOWER_ALPHA_NON_HEX_NO_P    \
  case 'g':                         \
  case 'h':                         \
  case 'i':                         \
  case 'j':                         \
  case 'k':                         \
  case 'l':                         \
  case 'm':                         \
  case 'n':                         \
  case 'o':                         \
  case 'q':                         \
  case 'r':                         \
  case 's':                         \
  case 't':                         \
  case 'u':                         \
  case 'v':                         \
  case 'w':                         \
  case 'x':                         \
  case 'y':                         \
  case 'z':

#define LOWER_ALPHA_NON_HEX   \
  case 'p':                   \
  LOWER_ALPHA_NON_HEX_NO_P    \

#define LOWER_ALPHA   \
  LOWER_ALPHA_HEX     \
  LOWER_ALPHA_NON_HEX

#define UPPER_ALPHA_NON_HEX_NO_P    \
  case 'G':                         \
  case 'H':                         \
  case 'I':                         \
  case 'J':                         \
  case 'K':                         \
  case 'L':                         \
  case 'M':                         \
  case 'N':                         \
  case 'O':                         \
  case 'Q':                         \
  case 'R':                         \
  case 'S':                         \
  case 'T':                         \
  case 'U':                         \
  case 'V':                         \
  case 'W':                         \
  case 'X':                         \
  case 'Y':                         \
  case 'Z':

#define UPPER_ALPHA_NON_HEX   \
  case 'P':                   \
  UPPER_ALPHA_NON_HEX_NO_P    \

#define UPPER_ALPHA   \
  UPPER_ALPHA_HEX     \
  UPPER_ALPHA_NON_HEX

#define ALPHA \
  LOWER_ALPHA \
  UPPER_ALPHA

#define ALPHA_NON_HEX \
  LOWER_ALPHA_NON_HEX \
  UPPER_ALPHA_NON_HEX

#define ALPHA_NON_HEX_NO_P \
  LOWER_ALPHA_NON_HEX_NO_P \
  UPPER_ALPHA_NON_HEX_NO_P \

// Exclusion of floating point scientific notation characters 'e', 'E', 'p', and 'P'.
#define ALPHA_NON_SCI \
  case 'a':           \
  case 'b':           \
  case 'c':           \
  case 'd':           \
  case 'f':           \
  case 'g':           \
  case 'h':           \
  case 'i':           \
  case 'j':           \
  case 'k':           \
  case 'l':           \
  case 'm':           \
  case 'n':           \
  case 'o':           \
  case 'q':           \
  case 'r':           \
  case 's':           \
  case 't':           \
  case 'u':           \
  case 'v':           \
  case 'w':           \
  case 'x':           \
  case 'y':           \
  case 'z':           \
  case 'A':           \
  case 'B':           \
  case 'C':           \
  case 'D':           \
  case 'F':           \
  case 'G':           \
  case 'H':           \
  case 'I':           \
  case 'J':           \
  case 'K':           \
  case 'L':           \
  case 'M':           \
  case 'N':           \
  case 'O':           \
  case 'Q':           \
  case 'R':           \
  case 'S':           \
  case 'T':           \
  case 'U':           \
  case 'V':           \
  case 'W':           \
  case 'X':           \
  case 'Y':           \
  case 'Z':

#define ALPHA_NO_E \
  case 'p':        \
  case 'P':        \
  ALPHA_NON_SCI

// Exclusion of characters 'b', 'B', 'o', 'O', 'x', and 'X'.
#define ALPHA_NON_RADIX \
  case 'a':             \
  case 'c':             \
  case 'd':             \
  case 'e':             \
  case 'f':             \
  case 'g':             \
  case 'h':             \
  case 'i':             \
  case 'j':             \
  case 'k':             \
  case 'l':             \
  case 'm':             \
  case 'n':             \
  case 'p':             \
  case 'q':             \
  case 'r':             \
  case 's':             \
  case 't':             \
  case 'u':             \
  case 'v':             \
  case 'w':             \
  case 'y':             \
  case 'z':             \
  case 'A':             \
  case 'C':             \
  case 'D':             \
  case 'E':             \
  case 'F':             \
  case 'G':             \
  case 'H':             \
  case 'I':             \
  case 'J':             \
  case 'K':             \
  case 'L':             \
  case 'M':             \
  case 'N':             \
  case 'P':             \
  case 'Q':             \
  case 'R':             \
  case 'S':             \
  case 'T':             \
  case 'U':             \
  case 'V':             \
  case 'W':             \
  case 'Y':             \
  case 'Z':

#endif
