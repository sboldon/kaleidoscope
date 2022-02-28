# Kaleidoscope

An implementation of a simple language using LLVM for code generation.

---

Outline for parser/lexer interaction. Parser has lexer and srcfile as members. lexer has
line offsets as a member. Parser has an error list where each error is a struct containing a span
and a message. Parser passes error list to lexer.

---

##### Development Resources:

* [My First Language Frontend with LLVM Tutorial](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)
