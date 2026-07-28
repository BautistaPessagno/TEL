---
name: writing-tel
description: Use when writing or generating TEL source code (the .tel language in this repo that compiles to C to reduce LLM token usage), translating an algorithm or program into TEL, or creating/editing .tel files.
---

# Writing TEL

## Overview

TEL is a Python-like, indentation-based language that compiles to C. Its whole purpose is to **use fewer tokens than the equivalent C**. So **write TEL, not C** — emitting C for program logic wastes the tokens TEL exists to save. Inline C blocks exist only as a deliberate escape hatch (e.g. calling `printf`).

## How TEL differs from C / Python

These are the error-prone rules. Get these right and most programs are correct.

| Rule | TEL |
|------|-----|
| Declarations | `name:type` — `x:int = 42` (NOT `int x = 42`) |
| Functions | `fn name p1:t1 p2:t2 -> ret`; body indented |
| `main` | bare `main` then indented body — no signature |
| Blocks | indentation, no braces; tab = 4 spaces |
| Call args | comma-separated: `f(a, b)` |
| Array literals | space-separated: `{1 2 3}` (empty `{}`) |
| Return | `ret expr`; final bare identifier / comparison auto-returns; call results need `ret` |
| Keywords | `ret`, `elif`, `cnt` (continue), `dw` (do-while), `ford` (counting for), `stc` (static) |
| Struct as a type | `b:struct Box` — keep the `struct`/`union`/`enum` word (or `typedef` an alias first) |
| Counting loop | `ford i 0 n` = `for(int i=0;i<n;i++)` |
| General loop | `for i = 0, i < n, i++` (comma-separated parts) |
| switch case | `1:` block (C fall-through) vs `1 -> expr` (auto-break); `default break` |
| Include | `#include stdio` → `<stdio.h>`; `null` = null pointer |
| Logical ops | `&& || !` (NOT `and`/`or`/`not`) |

## Template

A correct program to adapt:

```
fn sumTo n:int -> int
    total:int = 0
    ford i 0 n
        total += i
    ret total

main
    x:int = sumTo(10)
    if x > 40
        ret 0
    elif x == 0
        ret 1
    else
        ret 2
```

For anything beyond this — types, pointers, structs/unions/enums/typedefs, function pointers, operators, literals, preprocessor, inline C — **read `language-reference.md` in this skill directory** before writing.

## Validate a program

The compiler runs in Docker. Build once, then run your `.tel` file:

```
docker compose run --rm compiler src/main/bash/build.sh
docker compose run --rm compiler src/main/bash/run.sh path/to/program.tel
```

Exit 0 means the program passed syntax and semantic validation; `run.sh`
emits deterministic C on stdout. Named `.c` inputs translate in the opposite
direction to canonical TEL, and stdin C is available with `tel --from c`.

## Common mistakes

| Mistake | Fix |
|---------|-----|
| `int x = 5` | `x:int = 5` |
| `{ ... }` braces | indent the block |
| `{1, 2, 3}` | `{1 2 3}` |
| `f(a b)` | `f(a, b)` |
| `continue` / `do` / `else if` / `static` | `cnt` / `dw` / `elif` / `stc` |
| expecting a final `f(x)` to return | use `ret f(x)` |
| `x:int[]` as a variable | unsized arrays only as a parameter type |
| `b:Box` for a struct | `b:struct Box`, or `typedef Box:struct Box` then `b:Box` |
| writing C for the logic | write TEL; inline C is a last-resort escape hatch |
| `#include <stdio.h>` in an inline C block | use the native directive `#include stdio` |
| `#define N 8` then using `N` in code | semantic analysis does NOT expand macros — a macro name in an expression is an "unknown identifier"; use the literal value instead |
