# Kaleidoscope

An implementation of a simple language (with the goal of) using LLVM for code generation.
Originally, I began this project as a quick way to learn the fundamentals of LLVM, but I became
wrapped up in implementing the start of a module system, extensible error reporting, and the
tokenization of numeric literals in binary, octal, decimal, and hexadecimal form.

I am unsure of the final shape that the language will take. In the frontend tutorial that inspired
this project, Kaleidoscope has 64-bit floats as its sole datatype and therefore requires no type
annotations. I would like to add support for integer and string types, but I am unsure if the
language will be dynamically or statically typed. Additionally, I envision an OCaml-like module
system for the language where each file is a module and each module has a signature that defines its
public interface.

#### Dependencies

* Compiler that supports C++20
* [fmt](https://github.com/fmtlib/fmt) >= 8.1.1
* LLVM development libraries (not yet required to build)

#### Grammar (in progress)

```
<expression>          ::= <add-expression>

<add-expression>      ::= <multiply-expression> (("+" | "-") <multiply-expression)*

<multiply-expression> ::= <prefix-expression> (("*" | "/") <prefix-expression>)*

<prefix-expression>   ::= ("-" | "!") <primary-expression>

<primary-expression>  ::= <ident>
                        | <number>
                        | "(" <expression> ")"

<ident> ::= (<alpha> | "_") (<alpha> | "_" | <dec>)*
<alpha> ::= "a" .. "z" | "A" .. "Z"

<number>      ::= <int-lit> | <float-lit>
<float-lit>   ::= <dec-int> "." <dec-int>          [("e" | "E") ["+" | "-"] <dec-int>]
                | <hex-int> "." <hex> (["_"] hex)* [("p" | "P") ["+" | "-"] <dec-int>]
                | <dec-int> ("e" | "E") ["+" | "-"] <dec-int>
                | <hex-int> ("p" | "P") ["+" | "-"] <dec-int>
<int-lit>     ::= <dec-int>
                | <hex-int>
                | <oct-int>
                | <bin-int>

<dec-int> ::= <dec> (["_"] <dec>)*
<hex-int> ::= "0" ("x" | "X") (["_"] <hex>)+
<oct-int> ::= "0" ("o" | "O") (["_"] <oct>)+
<bin-int> ::= "0" ("b" | "B") (["_"] <bin>)+

<hex> ::= <dec> | "a" .. "f" | "A" .. "F"
<dec> ::= "0" .. "9"
<oct> ::= "0" .. "7"
<bin> ::= "0" | "1"
```
---

##### Development Resources:

* [My First Language Frontend with LLVM Tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)
