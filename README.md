[![Pipeline](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml)

# TEL

TEL is a Python-like DSL that compiles to C. The idea is to use a syntax that reduces LLMs token usage

This repository delivers the Stage 2 frontend: the executable reads TEL source from standard input, tokenizes it with Flex, parses it with Bison, builds an AST, and exits with success or failure.

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
- statements for declarations, returns, expressions, conditionals, loops, switches, `break`.
- primitive, named aggregate, typedef, pointer, array, and function-pointer types
- integer, floating-point, character, string, array, and `null` literals
- C-like unary, binary, assignment, bitwise, logical, member-access, and array-index expressions
- comma-separated function-call arguments
- AST allocation and recursive cleanup

For a complete description of TEL's syntax, keywords, and all changes from standard C and Stage 1, see [doc/requirements.md](doc/requirements.md).


## Tests

Tests are plain TEL programs under `src/test/c/accept` and `src/test/c/reject`.

- Accept tests must exit with status `0`.
- Reject tests must exit with a non-zero status.
- Test names use `NN-description`.
- Numeric prefixes must be unique within each directory.


## Configuration

The Docker service reads these optional environment variables:

| Name                  | Default | Description                                                  |
| :-------------------- | :------ | :----------------------------------------------------------- |
| `ENVIRONMENT`         | `Local` | Active environment name.                                     |
| `LOG_IGNORED_LEXEMES` | `true`  | Logs ignored Flex lexemes at `DEBUGGING` level when enabled. |
| `LOGGING_LEVEL`       | `ALL`   | Minimum console log level.                                   |
