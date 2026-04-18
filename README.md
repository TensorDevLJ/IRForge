# Mini C-Like Compiler

A complete mini compiler written in C++17 that processes a simple C-like language and demonstrates core compiler design concepts, including intermediate representation (IR) and optimization passes — aligned with how real GPU compilers (NVPTX / AMDGPU) and LLVM-based toolchains work internally.

---

## Features

| Phase | Module | What it does |
|-------|--------|-------------|
| 1 | Lexer | Tokenises source text into typed tokens |
| 2 | Parser | Recursive-descent parser; enforces grammar + operator precedence |
| 3 | AST | Builds a typed Abstract Syntax Tree |
| 4 | Symbol Table | `unordered_map`-backed variable store |
| 5 | Interpreter | Tree-walking evaluator; produces correct runtime values |
| 6 | IR Generator | Emits Three-Address Code **and** LLVM-style SSA IR |
| 7 | Optimizer | Constant folding · Copy propagation · Dead code elimination · Identity simplification |
| 8 | Error Handling | Meaningful errors at every phase (lexer, parser, runtime) |

---

## Language Supported

```
// Variable assignment
a = 10;

// Arithmetic with operator precedence (* / before + -)
b = 2 + 3 * 4;         // b = 14

// Parentheses
c = (2 + 3) * 4;       // c = 20

// Multiple statements
a = 5; b = a + 2; c = b * 3;

// If statement (optional extension)
if (x > 5) { y = 1; } else { y = 0; }
```

---

## Build

**Requirements:** `g++` with C++17 support (GCC 7+ or Clang 5+).

```bash
make          # compile
make run      # compile and run all tests
make clean    # remove build artefacts
```

Manual build (no Make):
```bash
g++ -std=c++17 -Wall -O2 -I src \
    src/lexer.cpp src/parser.cpp src/ast.cpp \
    src/interpreter.cpp src/ir_generator.cpp \
    src/optimizer.cpp src/main.cpp \
    -o mini_compiler

./mini_compiler
```

---

## Project Structure

```
mini_compiler/
├── Makefile
├── README.md
└── src/
    ├── lexer.h / lexer.cpp          Phase 1: Lexical Analysis
    ├── ast.h   / ast.cpp            Phase 3: AST node types
    ├── parser.h / parser.cpp        Phase 2: Recursive-descent parser
    ├── symbol_table.h               Phase 4: Variable storage
    ├── interpreter.h / interpreter.cpp  Phase 5: Tree-walking eval
    ├── ir_generator.h / ir_generator.cpp  Phase 6: TAC + LLVM IR
    ├── optimizer.h / optimizer.cpp  Phase 7: Optimisation passes
    └── main.cpp                     Driver – 10 test cases
```

---

## Example Output

**Input:** `a = 2 + 3 * 4;`

```
PHASE 1 — LEXICAL ANALYSIS
  IDENTIFIER("a") @1:2
  EQUAL("=") @1:4
  NUMBER("2") @1:6
  PLUS("+") @1:8
  NUMBER("3") @1:10
  MUL("*") @1:12
  NUMBER("4") @1:14
  SEMICOLON(";") @1:15

PHASE 2+3 — PARSER & AST
  PROGRAM
    ASSIGN(a)
      BINARY_OP(+)
        NUMBER(2)
        BINARY_OP(*)
          NUMBER(3)
          NUMBER(4)

PHASE 5 — INTERPRETER
  >> a = 14

PHASE 6 — IR GENERATION
Three-Address Code:
  t1 = 3 * 4
  t2 = 2 + t1
  a = t2

LLVM-style IR:
  %1 = mul i32 3, 4
  %2 = add i32 2, %1
  store i32 %2, ptr @a

PHASE 7 — OPTIMIZATION
Optimized IR:
  a = 14
Optimization Stats:
  Constant folds: 2
  Dead instructions: 1
```

---

## GPU / LLVM Connection

The LLVM-style IR emitted by `ir_generator` directly mirrors the format consumed by:
- **NVPTX backend** (NVIDIA GPU target in LLVM)
- **AMDGPU backend** (AMD ROCm / HIP)

Our `mul i32` / `add i32` instructions map 1-to-1 to PTX scalar operations, making this project a direct conceptual bridge to GPU compiler internals.

---

## Optimization Passes

| Pass | Description | Example |
|------|-------------|---------|
| Constant Folding | Both args literal → compute at compile time | `t1 = 3 * 4` → `t1 = 12` |
| Copy Propagation | Substitute known constants forward | `t2 = 2 + t1` → `t2 = 14` |
| Dead Code Elim. | Remove writes never read before overwrite | `a = 5; a = 10;` → `a = 10` |
| Identity Simplification | Arithmetic identities | `x * 1` → `x`, `x + 0` → `x` |

---

## Error Handling

The compiler provides clear, located error messages:

```
[Lexer Error]  Unexpected character '@' at line 1, col 5
[Parser Error] Expected ';' at end of assignment to 'a' (got IDENTIFIER("b") @1:7)
[Runtime Error] Undefined variable: 'z'
[Runtime Error] Division by zero
```
