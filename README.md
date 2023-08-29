# LBASIC

A simple, compiled language with syntax inspired by C and BASIC

### Dependencies:
- Some reasonably current version of GCC (v10 or greater, perhaps)
- clang-format-11

### Progress:
- [x] Lexer
- [x] Parser
- [ ] Type Checker (in-progress)
- [ ] Code Generator

### Planned Features:
- Static type system
- C-like structures
- x86 target architecture

### To Build:
Just run `make`.

### To Install:
Run `make install` to install the `lbasic` binary into `/usr/local/bin` (requires root access).

### To run source code formatter (clang-format-11):
Run `make format`.
