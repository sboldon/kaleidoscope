/* C++ code produced by gperf version 3.1 */
/* Command-line: gperf --multiple-iterations=100 keywords.gperf  */
/* Computed positions: -k'' */

#line 12 "keywords.gperf"

#include "token.hpp"
#line 15 "keywords.gperf"
struct keyword_hash_entry { const char* name; token::type kind; };

#define TOTAL_KEYWORDS 2
#define MIN_WORD_LENGTH 3
#define MAX_WORD_LENGTH 6
#define MIN_HASH_VALUE 3
#define MAX_HASH_VALUE 6
/* maximum key range = 4, duplicates = 0 */

class perfect_hash
{
private:
  static inline unsigned int hash (const char *str, size_t len);
public:
  static const inline token::type get_token (const char *str, size_t len);
};

inline /*ARGSUSED*/
unsigned int
perfect_hash::hash (const char *str, size_t len)
{
  return len;
}

static const unsigned char lengthtable[] =
  {
     0,  0,  0,  3,  0,  0,  6
  };

static const struct keyword_hash_entry wordlist[] =
  {
    {"",token::type::IDENT}, {"",token::type::IDENT},
    {"",token::type::IDENT},
#line 18 "keywords.gperf"
    {"def", token::type::DEF},
    {"",token::type::IDENT}, {"",token::type::IDENT},
#line 19 "keywords.gperf"
    {"extern", token::type::EXTERN}
  };

const inline token::type 
perfect_hash::get_token (const char *str, size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == lengthtable[key])
          {
            const char *s = wordlist[key].name;

            if (*str == *s && !memcmp (str + 1, s + 1, len - 1))
              return wordlist[key].kind;
          }
    }
  return token::type::IDENT;
}
