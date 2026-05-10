[![Pipeline](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml)

# TEL

TEL is a Python-like DSL that compiles to C. This repository delivers the Stage 2 frontend: the executable reads TEL source from standard input, tokenizes it with Flex, parses it with Bison, builds an AST, and exits with success or failure. C code generation, full semantic validation, type checking, and automatic include insertion are Stage 3 work.

## Team

- Bautista Pessagno
- Lorenzo Alejandro Mendez
- Rodrigo Alejandro Hernandez

## Requirements

- Docker v28 or newer

All normal build and test commands should run inside the Docker Compose service so generated Linux artifacts match the grading environment.

## Commands

Start an interactive development container:

```bash
docker compose run --rm compiler
```

Build inside the container:

```bash
src/main/bash/build.sh
```

Run a TEL program inside the container:

```bash
src/main/bash/run.sh program.tel
```

Run the Stage 2 test suite inside the container:

```bash
src/main/bash/test.sh
```

From the host, the same commands can be run through Docker:

```bash
docker compose run --rm compiler src/main/bash/build.sh
docker compose run --rm compiler src/main/bash/test.sh
```

Do not use the host shell as the final verification signal on macOS. `.build/Flex-Bison-Compiler` is a Linux binary produced for the container.

## Stage 2 Scope

Implemented for this handoff:

- top-level variable declarations, function declarations, `main`, structs, unions, enums, typedefs, preprocessor directives, comments, and inline C blocks
- indentation-based bodies using `INDENT` and `DEDENT`
- statements for declarations, returns, expressions, conditionals, loops, switches, `break`, and `cnt`
- primitive, named aggregate, typedef, pointer, array, and function-pointer types
- integer, floating-point, character, string, array, and `null` literals
- C-like unary, binary, assignment, bitwise, logical, member-access, and array-index expressions
- comma-separated function-call arguments
- AST allocation and recursive cleanup

Deferred to Stage 3:

- C code generation and runtime output
- complete type checking and semantic validation
- automatic `stdio` includes for I/O usage
- type inference beyond syntax accepted by the frontend
- semantic rejection of otherwise parseable programs

## Language at a Glance

### Types

| TEL      | C                   |
| :------- | :------------------ |
| `int`    | `int`               |
| `char`   | `char`              |
| `float`  | `float`             |
| `double` | `double`            |
| `long`   | `long`              |
| `void`   | `void`              |
| `uint`   | `unsigned int`      |
| `uli`    | `unsigned long int` |

### Declarations and functions

```tel
x:int = 5
arr:int[3] = {1, 2, 3}

fn double x:int -> int
    ret x * 2

fn greet name:char* -> void
    log(name)
```

Return type may be omitted for `void` functions. Function-call arguments are comma-separated: `add(x, 2)`.

### Indentation

Blocks are delimited by indentation (multiples of 4 spaces, like Python) instead of `{}`.

### Control flow

```tel
if x > 0
    ret x
elif x == 0
    ret 0
else
    ret -1

ford i 0 n          // for (int i = 0; i < n; i++)
    log(i)

for i = 0, i < n, i++
    log(i)

while cond
    step()

dw
    step()
cond

switch value
    1: do_one()     // fall-through with ':'
    2: do_two()
       break
    3 -> do_three() // auto-break with '->'
    default: do_default()
```

### Aggregates and typedefs

```tel
struct Point
    x:int
    y:int

enum Color
    Red
    Green
    Blue

typedef Byte: uint
```

### Preprocessor and inline C

```tel
#include stdio
#define MAX 100

`
int raw = MAX * 2;
`
```

### Comments

```tel
// single-line comment
/* multi-line
   comment */
```

### Example program

```tel
fn add a:int b:int -> int
    ret a + b

main
    x:int = 10
    y:int = 32
    result:int = add(x, y)
    ret result
```

## Tests

Tests are plain TEL programs under `src/test/c/accept` and `src/test/c/reject`.

- Accept tests must exit with status `0`.
- Reject tests must exit with a non-zero status.
- Test names use `NN-description`.
- Numeric prefixes must be unique within each directory.

## Project Layout

```
src/main/c/
  frontend/
    lexical-analysis/     Flex lexer (FlexPatterns.l, FlexActions.c)
    syntactic-analysis/   Bison parser (BisonGrammar.y, BisonActions.c, AbstractSyntaxTree.c/h)
  EntryPoint.c            compiler driver
src/test/c/
  accept/                 programs the compiler must accept (exit 0)
  reject/                 programs the compiler must reject (exit ≠ 0)
doc/
  next_steps.md           Stage 2 development plan and completion criteria
TLA stage 2/              course specification PDFs
```

## Configuration

The Docker service reads these optional environment variables:

| Name                  | Default | Description                                                  |
| :-------------------- | :------ | :----------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | Active environment name.                                     |
| `LOG_IGNORED_LEXEMES` | `true`  | Logs ignored Flex lexemes at `DEBUGGING` level when enabled. |
| `LOGGING_LEVEL`       | `ALL`   | Minimum console log level.                                   |
