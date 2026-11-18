# lex_cpp: Modern C++ Lexical Analyzer Generator

A fast, lightweight, and robust **Lexical Analyzer Generator** (Lex / Flex equivalent) written in modern **C++20**, designed strictly adhering to the style, philosophy, and guidelines of C++ creator **Bjarne Stroustrup** (*The C++ Programming Language*, *A Tour of C++*, and *The C++ Core Guidelines*).

---

## 1. Design Philosophy: "Founder of C++" Style

The architecture and implementation embody Bjarne Stroustrup's core design tenets:

1. **Direct Representation of Concepts**:
   - Fundamental compiler concepts are concrete, well-typed classes: [`Char_set`](include/lex/char_set.hpp), [`Nfa`](include/lex/nfa.hpp), [`Dfa`](include/lex/dfa.hpp), [`Regex_node`](include/lex/regex.hpp), [`Spec_parser`](include/lex/spec_parser.hpp), and [`Lexer_generator`](include/lex/lexer_generator.hpp).
   - Type-safe representations: strongly typed IDs (`State_id`, `Rule_id`), scoped enumerations (`enum class Regex_kind`), bitsets (`std::bitset<256>`).
2. **Resource Management via RAII**:
   - Zero raw `new` and `delete` across the entire codebase.
   - Deterministic resource lifetimes through `std::unique_ptr` for polymorphic ASTs and standard containers (`std::vector`, `std::string`, `std::map`).
   - Rule of Zero: classes manage members cleanly without redundant manual copy/move logic.
3. **Value Semantics & Immutability**:
   - Fast, value-oriented types (e.g. `Char_set` with bitwise set operators `|`, `&`, `~`).
   - `std::string_view` for zero-allocation views into source buffers.
   - Strict `const`-correctness and `noexcept` specifications on non-throwing operations.
4. **Standard Library Integration**:
   - Standard algorithms and containers used instead of proprietary ad-hoc structures.
   - Zero external third-party dependencies.
5. **No Enterprise OOP Over-Abstraction**:
   - Avoids "Java-in-C++" antipatterns. Concrete types with direct invariants are favored; polymorphism is reserved strictly for syntax tree variants.

---

## 2. Architecture & Pipeline

```
  .l File ────> [ Spec_parser ] ────> Definitions, Rules & Actions
                       │
                       ▼
              [ Regex_parser ] ────> Regex AST
                       │
                       ▼
             [ Thompson_builder ] ──> Combined NFA (Rule Priority Tagged)
                       │
                       ▼
               [ Dfa_builder ] ─────> Subset Construction (Powerset DFA)
                       │
                       ▼
             [ Dfa_minimizer ] ────> Hopcroft / Moore Partition Refinement
                       │
                       ▼
             [ Code_generator ] ───> Generated C++ Scanner (Maximal Munch)
```

- **Maximal Munch (Longest Match)**: The generated scanner advances greedily, recording the most recent accepting state and input position, backtracking if a dead state is hit.
- **Rule Precedence Resolution**: When multiple rules match prefixes of equal length, the rule appearing earliest in the specification file wins.
- **Automata Minimization**: Discards unreachable states and refines partitions to yield minimal state counts and compact transition tables.

---

## 3. Building

### Prerequisites
- Modern C++20 compiler (`g++` 10+, `clang++` 10+)
- CMake 3.20+
- Make or Ninja

```bash
cmake -B build
cmake --build build
```

Run test suite:
```bash
./build/test_runner
```

---

## 4. CLI Usage

```bash
./build/lex_cpp [options] <input.l>
```

### Options:
| Option | Description |
|---|---|
| `-o <file>` | Output generated C++ source file (default: `lex.yy.cpp`) |
| `--header <file>` | Output generated C++ header file (optional) |
| `--class-name <name>` | Scanner class name (default: `Lexer`) |
| `--namespace <name>` | C++ namespace wrapping the generated scanner (optional) |
| `--dot-nfa <file>` | Export combined NFA graph in Graphviz DOT format |
| `--dot-dfa <file>` | Export minimized DFA graph in Graphviz DOT format |
| `-v, --verbose` | Print NFA/DFA compilation and state minimization statistics |
| `-h, --help` | Display help information |

---

## 5. Examples

### Calculator Scanner ([`examples/calc.l`](examples/calc.l))
```bash
# 1. Generate scanner
./build/lex_cpp examples/calc.l -o calc_lexer.cpp --verbose

# 2. Compile generated C++ scanner
g++ -std=c++20 -Wall -Wextra -Wpedantic calc_lexer.cpp -o calc_lexer

# 3. Execute
./calc_lexer
```

Output:
```
Input expression: total = 42 + 3.14159 * (radius - 10);

Tokens:
[ID: total]
[ASSIGN]
[INT: 42]
[PLUS]
[FLOAT: 3.14159]
[MUL]
[LPAREN]
[ID: radius]
[MINUS]
[INT: 10]
[RPAREN]
[SEMICOLON]

Lexical analysis completed successfully.
```

### C++ Tokenizer ([`examples/cpp_tokens.l`](examples/cpp_tokens.l))
```bash
./build/lex_cpp examples/cpp_tokens.l -o cpp_tokens_lexer.cpp
g++ -std=c++20 cpp_tokens_lexer.cpp -o cpp_tokens_lexer
./cpp_tokens_lexer
```
