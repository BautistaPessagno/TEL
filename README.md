[![Pipeline](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml/badge.svg?branch=development)](https://github.com/BautistaPessagno/TEL/actions/workflows/pipeline.yaml)

# TEL

TEL is a Python-like DSL for representing a supported subset of C with fewer
LLM tokens. The compiler translates in both directions.

Named inputs select the direction from their lowercase extension:

```text
tel program.tel                 # TEL -> C on stdout
tel program.c                   # C -> canonical TEL on stdout
tel program.tel -o program.c
tel program.c -o program.tel
```

For compatibility, stdin defaults to TEL. Use `--from c` for C stdin:

```bash
tel < program.tel
tel --from tel < program.tel
tel --from c < program.c
```

The compiler rejects unknown or uppercase extensions, and `--from` cannot
override a named file. Generation is buffered; `-o` uses a temporary sibling
and atomic rename, so a rejected input cannot truncate an existing output.

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

Translate a named TEL or C program inside the container:

```bash
src/main/bash/run.sh program.tel
src/main/bash/run.sh program.c
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
- a separately prefixed, reentrant Flex/Bison frontend for the supported C
  subset
- deterministic TEL generation with precedence-aware expressions, compact
  types, canonical `ford` loops, switch arrows, `null`, and `0o` octal literals
- parser-neutral AST builders and post-validation normalization
- marker-delimited, round-trip-safe inline C

## Canonical Bijection

Formatting, ordinary comments, redundant parentheses, repeated compatible
prototypes, and equivalent declaration ordering are normalized. The formal
bijection is over canonical compiler outputs:

```text
CToTEL(TELToC(t)) = canonicalTEL(t)
TELToC(CToTEL(c)) = canonicalC(c)
```

Handwritten supported C is accepted as a normalization extension. The C
frontend covers the TEL AST boundary: supported declarations and declarators,
functions and common `main` signatures, aggregates/enums/typedefs, initializer
lists, expressions, control flow, system includes, opaque defines, and inline-C
markers. It rejects non-representable C rather than approximating it.

See [the bidirectional compiler contract](doc/bidirectional-compiler.md) for
the precise supported and unsupported C surface.


TEL relies on the C standard library, user includes, inline C, and the C
toolchain. It does not provide a separate runtime.

For the language surface, see [the final report](doc/informe_final.md). For the
C boundary and canonical round-trip contract, see
[the bidirectional compiler contract](doc/bidirectional-compiler.md).


## Tests

TEL acceptance/rejection tests live under `src/test/c/accept` and
`src/test/c/reject`. Bidirectional tests additionally include:

- `src/test/c/generate`: TEL-to-C golden files
- `src/test/c/translate`: supported C-to-TEL golden files
- `src/test/c/reject-c`: unsupported C with filename/line diagnostics
- `src/test/bash/test-roundtrip.sh`: both canonical inverse laws

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
