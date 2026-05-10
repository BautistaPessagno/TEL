[![Pipeline](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml)

# TEL

TEL is a Flex/Bison compiler frontend for a Python-like language that targets C in later stages.

This repository is prepared for the Stage 2 handoff: the executable reads TEL source from standard input, tokenizes it with Flex, parses it with Bison, builds an AST, and exits with success or failure. C code generation, complete semantic validation, type checking, and automatic include insertion are Stage 3 work.

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

## Tests

Tests are plain TEL programs under `src/test/c/accept` and `src/test/c/reject`.

- Accept tests must exit with status `0`.
- Reject tests must exit with a non-zero status.
- Test names use `NN-description`.
- Numeric prefixes must be unique within each directory.

Current Stage 2 matrix:

| Requirement group | Accept tests | Reject tests |
| :-- | :-- | :-- |
| Declarations, types, initializers, terminators | `01`-`03` | `01`-`04` |
| Functions, `main`, returns, implicit returns | `04`-`07` | `05`-`08` |
| Calls, expressions, operators | `08`-`10` | `09`-`12` |
| Indentation, blocks, program shape | `11`-`12` | `13`-`16` |
| Control flow | `13`-`16` | `17`-`20` |
| Aggregates, typedefs, named types | `17`-`19` | `21`-`24` |
| Preprocessor, comments, inline C | `20`-`22` | `25`-`28` |
| Literals | `23`-`25` | `29`-`31` |
| Pointers, arrays, function pointers, member access | `26`-`29` | `32`-`35` |
| Integration programs | `30`-`32` | `36`-`38` |

## Configuration

The Docker service reads these optional environment variables:

| Name | Default | Description |
| :-- | :-- | :-- |
| `ENVIRONMENT` | `Local` | Active environment name. |
| `LOG_IGNORED_LEXEMES` | `true` | Logs ignored Flex lexemes at `DEBUGGING` level when enabled. |
| `LOGGING_LEVEL` | `ALL` | Minimum console log level. |
