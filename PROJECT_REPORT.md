# PROJECT REPORT: MODERN C++ LEXICAL ANALYZER GENERATOR (LEX)

**Project Title**: Design and Implementation of a Lexical Analyzer Generator in Modern C++  
**Author**: Gulshan Sharma  
**Target Environment**: Linux, C++20, GCC 14 / Clang  
**Repository**: [https://github.com/Gulshan1-3/lex](https://github.com/Gulshan1-3/lex)  
**Interactive Live Demo**: [https://gulshan1-3.github.io/lex/](https://gulshan1-3.github.io/lex/)  

---

## Executive Submission Summary & Links

* **Live Demo Link (Online Playground)**:  
  [https://gulshan1-3.github.io/lex/](https://gulshan1-3.github.io/lex/)  
  *(Interactive browser-based playground to load Lex specifications, test custom inputs, observe real-time token streams, and inspect DFA state graphs)*

* **GitHub Repository Link**:  
  [https://github.com/Gulshan1-3/lex](https://github.com/Gulshan1-3/lex)  
  *(Contains complete source code, test suite, CMake build system, examples, and documentation)*

---

## 1. Project Overview

Lexical analysis (scanning) is the foundational first phase of any compiler or language processing system. A **Lexical Analyzer Generator** (such as classic `lex` or `flex`) accepts high-level regular expression specifications paired with semantic action code and automatically synthesizes an optimized, deterministic scanner state machine.

This project implements a complete, standalone Lexical Analyzer Generator (`lex_cpp`) written in **ISO C++20**. The software architecture and implementation strictly adhere to the principles and idioms championed by **Bjarne Stroustrup**, the creator of C++:
* Direct representation of domain concepts via strong types.
* Strict Resource Acquisition Is Initialization (RAII) with zero manual memory management (`new`/`delete`).
* Value semantics and standard library integration (`std::vector`, `std::string_view`, `std::bitset<256>`).
* Minimalist, robust concrete classes without excessive OOP inheritance hierarchies ("no Java-in-C++").

---

## 2. Core Architectural Pipeline

The generation pipeline follows the classical compiler pipeline with modern optimizations:

```
  Input .l File
        |
        v
  [ Spec_parser ] ------> Parses Prologue %{...%}, Definitions, Rules, Epilogue
        |
        v
  [ Regex_parser ] -----> Constructs Abstract Syntax Tree (AST)
        |
        v
  [ Thompson_builder ] -> Synthesizes Combined NFA with Prioritized Rule Tags
        |
        v
  [ Dfa_builder ] ------> Subset Construction (Powerset & epsilon-closure)
        |
        v
  [ Dfa_minimizer ] ----> Hopcroft / Moore Partition Refinement (Prunes unreachable)
        |
        v
  [ Code_generator ] ---> Generates C++20 Lexer Class with Maximal Munch Buffering
```

### 2.1 Specification Parser (`Spec_parser`)
Parses the standard 3-section Lex specification format:
1. **Definitions Section**: Captures C++ header declarations (`%{ ... %}`) and named macro substitutions (e.g., `DIGIT [0-9]`).
2. **Rules Section**: Parses regular expression patterns mapped to action blocks `{ ... }`, accurately tracking brace depth and comment escapes. Handles rule fall-through syntax (`|`).
3. **Epilogue Section**: Preserves user-defined driver code (such as `main()`) copied verbatim to the generated output.

### 2.2 Regular Expression Parser (`Regex_parser`)
Implements a recursive-descent parser producing a strongly typed AST:
* **Primitives**: Literals, escape codes (`\n`, `\t`, `\r`, `\xHH`), wildcards (`.`), quoted strings (`"keyword"`).
* **Character Classes**: Explicit ranges (`[a-zA-Z0-9_]`), leading hyphens (`[-a-z]`), negated classes (`[^0-9]`).
* **Closures & Operators**: Kleene star (`*`), positive closure (`+`), optional quantifier (`?`), counted repetitions (`{m,n}`), concatenation, and alternation (`|`).
* **Macro Expansion**: Recursive expansion of named definitions (`{DIGIT}`).

### 2.3 Thompson's NFA Construction (`Thompson_builder`)
Transforms the AST into a Non-Deterministic Finite Automaton with $\epsilon$-transitions:
* Each rule $i$ is compiled into an NFA fragment.
* All rule NFAs are unified under a single master start state.
* The accepting state of each rule is tagged with its `Rule_id` and specification index to enforce rule precedence.

### 2.4 Subset Construction (`Dfa_builder`)
Performs powerset construction to convert the combined NFA into a Deterministic Finite Automaton (DFA):
* Calculates $\epsilon$-closure sets using depth-first search.
* Maps subsets of NFA states to unique DFA states.
* Resolves rule conflict: if a DFA state contains multiple NFA accepting states, the rule appearing earliest in the specification file wins.

### 2.5 DFA Minimization (`Dfa_minimizer`)
* Performs reachability analysis via Breadth-First Search to prune unreachable states.
* Employs partition refinement (Hopcroft/Moore):
  * Initial partitions group non-accepting states and separate accepting states by their distinct `Rule_id`.
  * Partitions are iteratively refined until all transitions within a group target identical partition blocks.
* Yields minimal state tables with reduced memory and cache footprint.

### 2.6 Modern C++ Code Generator (`Code_generator`)
Generates production-grade, warning-free ISO C++20 code:
* **Maximal Munch (Longest Match)**: The scanner greedily advances as far as possible while transitions exist, tracking the last visited accepting state and position. If a dead state is reached, it seamlessly rolls back to the longest accepted token boundary.
* **Dual Input Modes**: Seamlessly processes both streaming input (`std::istream&`) and zero-copy string buffers (`std::string_view`).
* **Standard Interface**: Provides `yytext`, `yyleng`, `yylineno`, `yycolumn`, and `yylex()`.

---

## 3. Adherence to Bjarne Stroustrup's C++ Principles

| Principle | Implementation in `lex_cpp` |
|---|---|
| **RAII & No Raw Pointers** | Memory is automatically managed by `std::unique_ptr` and standard containers. Zero `new` or `delete` keywords in application logic. |
| **Direct Concept Representation** | Concrete domain types: `Char_set`, `Nfa_state`, `Dfa_state`, `Rule`, `Spec_file`, `Lexer_generator`. |
| **Value Semantics** | Fast, copyable concrete types like `Char_set` built over `std::bitset<256>`. |
| **Strict Type Safety** | Scoped enumerations (`enum class Regex_kind`), strongly typed `State_id` and `Rule_id`. |
| **Standard Algorithms & Ranges** | Leverages `<algorithm>`, `<ranges>`, and `<bitset>` for clean, expressive logic. |
| **Zero Warnings** | Builds cleanly with `-Wall -Wextra -Wpedantic -Wconversion -Wshadow`. |

---

## 4. Verification & Testing

### 4.1 Automated Test Suite
The project includes a self-contained unit and integration test suite (`tests/test_runner`):
* **`test_char_set`**: Bitset operations, range construction, set union, intersection, negation.
* **`test_regex`**: Lexing, parsing, escape sequences, bracket classes, macro substitution.
* **`test_automata`**: Thompson NFA construction, subset DFA conversion, Hopcroft minimization, and rule priority.
* **`test_end_to_end`**: Specification parsing and end-to-end scanner generation.

**Result**: 63 / 63 tests passing with 0 failures.

### 4.2 Practical Examples

#### 1. Arithmetic Calculator Scanner (`examples/calc.l`)
Tokenizes complex expressions containing floating-point numbers, integers, identifiers, and parenthesized operators:
* Input: `total = 42 + 3.14159 * (radius - 10);`
* Reduction: 47 NFA states $\to$ 19 DFA states $\to$ **15 Minimized DFA states**.
* Successfully verified token stream and line/column positions.

#### 2. C++ Tokenizer (`examples/cpp_tokens.l`)
Tokenizes real-world C++ code constructs including keywords (`namespace`, `struct`, `int`, `return`), hex literals (`0x2A`), line comments, and multi-character operators (`->`, `>=`, `&&`, `!=`).
* Reduction: 315 NFA states $\to$ 100 DFA states $\to$ **58 Minimized DFA states**.
* Confirmed maximal munch priority (e.g. `->` matched as single operator rather than `-` followed by `>`).

---

## 5. Build and Execution Instructions

### Prerequisites
* GCC 10+ or Clang 10+ (supporting C++20)
* CMake 3.20+
* Make or Ninja

### Build Steps
```bash
# Clone the repository
git clone https://github.com/Gulshan1-3/lex.git
cd lex

# Configure and build
cmake -B build
cmake --build build

# Run automated tests
./build/test_runner
```

### Generating and Compiling a Scanner
```bash
# Generate C++ scanner from specification
./build/lex_cpp examples/calc.l -o calc_lexer.cpp --verbose

# Compile the generated scanner
g++ -std=c++20 -Wall -Wextra -Wpedantic calc_lexer.cpp -o calc_lexer

# Run scanner
./calc_lexer
```

---

## 6. Live Interactive Demo

An interactive Web demonstration is deployed on GitHub Pages:
* **Live Demo URL**: [https://gulshan1-3.github.io/lex/](https://gulshan1-3.github.io/lex/)
* **Features**:
  * Real-time lexical analysis in the browser.
  * Presets for Calculator (`calc.l`), C++ Tokenizer (`cpp_tokens.l`), and JSON Scanner (`json.l`).
  * Live Token Stream visualization with matched lexemes, lengths, and positions.
  * Automaton statistics (NFA states, DFA states, Minimized DFA states).
  * Live C++20 Scanner code viewer with copy functionality.
  * Graphviz DOT graph export.

---

## 7. Conclusion

The `lex_cpp` project demonstrates how classical compiler algorithms (Thompson's construction, subset powerset construction, Hopcroft partition refinement, and maximal munch scanning) can be implemented with elegance, optimal performance, and uncompromising type safety using modern ISO C++20 according to Bjarne Stroustrup's design guidelines.
