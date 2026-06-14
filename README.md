[![Pipeline](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml)

# TEL

TEL is a Python-like DSL that compiles to C. The idea is to use a syntax that reduces LLM token usage.

The compiler reads TEL from standard input, builds an AST with Flex/Bison,
performs semantic validation, and emits C to standard output.

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

Run the full test suite inside the container:

```bash
src/main/bash/test.sh
```

Run a TEL program inside the container:

```bash
src/main/bash/run.sh program.tel
```

Generate and compile a C translation unit:

```bash
src/main/bash/run.sh program.tel > program.c
gcc program.c -o program
```



From the host, the same commands can be run through Docker:

```bash
docker compose run --rm compiler src/main/bash/build.sh
docker compose run --rm compiler src/main/bash/test.sh
```

Do not use the host shell as the final verification signal on macOS. `.build/tel` is a Linux binary produced for the container.




## Skills

This repo ships a `writing-tel` skill that teaches Claude (or other agents supporting [Agent Skills](https://github.com/anthropics/skills)) how to write correct TEL source code.

Install it into your own project with:

```bash
npx skills add https://github.com/BautistaPessagno/TEL --skill writing-tel
```

## Implemented Scope

Implemented:

- top-level variable declarations, function declarations, `main`, structs, unions, enums, typedefs, preprocessor directives, comments, and inline C blocks
- indentation-based bodies using `INDENT` and `DEDENT`
- statements for declarations, returns, expressions, conditionals, loops, switches, `break`.
- primitive, named aggregate, typedef, pointer, array, and function-pointer types
- integer, floating-point, character, string, array, and `null` literals
- C-like unary, binary, assignment, bitwise, logical, member-access, and array-index expressions
- comma-separated function-call arguments
- AST allocation and recursive cleanup
- symbol tables, nested scopes, type compatibility, declarations, calls,
  assignments, returns, arrays, pointers, aggregates, and control-flow
  validation
- deterministic C generation for declarations, declarators, functions,
  expressions, control flow, directives, inline C, and `main`


TEL relies on the C standard library, user includes, inline C, and the C
toolchain. It does not provide a separate runtime.

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
