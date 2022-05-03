#!/usr/bin/env python
# --- Run when changes are made to keywords.gperf ---
# Change the gperf generated function `in_word_set` to be an inline function with a return type of
# token::type instead of a pointer to `keyword_hash_entry` struct. If the string is a keyword, the
# corresponding token will be returned, otherwise token::type::IDENT is returned. Inspiration for
# this gperf output modification comes from how V8 engine handles Javascript keywords.
import re
import subprocess
import sys

GPERF_INPUT = 'keywords.gperf'
OUTPUT = 'keyhash.hpp'

def validate_sub(pattern, replacement, string, count=1, flags=0):
    (updated_string, num_replacements) = re.subn(pattern, replacement, string, count, flags)
    if num_replacements != count:
        raise RuntimeError("Failed to replace pattern '%s' in %d places" % (pattern, count))
    return updated_string

def main():
    gperf_output = subprocess.check_output(['gperf', '--multiple-iterations=100', GPERF_INPUT])
    # Change return type.
    gperf_output = validate_sub(r'struct\s*keyword_hash_entry\s*\*',
                                'inline token::type ',
                                gperf_output,
                                count=2)
    # Change return values.
    gperf_output = validate_sub(r'&(wordlist\[key\])', r'\1.kind', gperf_output)
    gperf_output = validate_sub(r'(return\s*)0', r'\1token::type::ident', gperf_output)
    with open(OUTPUT, 'w') as f:
        f.write(gperf_output)
    return 0;


if __name__ == '__main__':
    sys.exit(main())
