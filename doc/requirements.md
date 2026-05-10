# Requirements

## Keywords

Single words are kept as-is, since they are single tokens.

- int -> int
- char -> char
- float -> float
- double -> double
- long -> long
- void -> void
- uint -> unsigned int
- uli -> unsigned long int

They are used like this:

`x:int` -> `int x`

## Function declarations

Functions are declared with `fn name args -> return_type`, where:

- `args` is a space-separated list of `name:type` pairs
- `-> return_type` declares the return type, and may be omitted if the function returns `void`

### Examples

A function returning `int`:

```c
int foo(int arg1, char arg2);
```

becomes:

```
fn foo arg1:int arg2:char -> int
```

A `void` function lets the return type be omitted:

```c
void bar(int x);
```

becomes:

```
fn bar x:int
```

### Function pointers

Function pointer types use `fn*`:

```
fn* int -> int        // pointer to a function taking int, returning int
fn* int int -> void   // pointer to a function taking two ints, returning void
fn*                   // pointer to a function taking no args, returning void
```

## Statement termination

Semicolons are optional at the end of a line. To place multiple statements on the same line, separate them with `;`.

## Return statements

Use `ret` to return a value from a function:

```
ret x + 1
ret 0
ret
```

If the last statement in a function body is a standalone identifier (e.g. `x`) or a standalone expression consisting only of comparison or logical operators (e.g. `a > b`, `x == y`, `!flag`), it is automatically treated as a `ret` statement. Explicit `ret` is still required for other expressions.

## Function calls

Function call arguments are comma-separated.

```
add(x, 2)
add(x + 1, 2 * 3)
pair(x, (y + 1))
```

Whitespace alone does not separate function arguments, so `add(x 2)` is invalid.

## Indentation

Instead of using `{}` for code blocks, we use indentation, like Python.

## Control flow

### Conditionals

- `if` -> `if`
- `else if` -> `elif`
- `else` -> `else`

### Loops

- `ford i 0 n` -> `for (int i = 0; i < n; i++)`
- `for i = 0, condition, i++` -> `for (int i = 0; condition; i++)`
- `while` -> `while`
- `dw` -> `do while`

### Switch

Two variants are supported, distinguished by the case separator.

**C-style (`:`) — fall-through:** cases fall through to the next one unless an explicit `break` is provided.

```
switch variable
    1: do_one()
    2: do_two()
       break
    3: do_three()
    default: do_default()
```

Here, case `1` falls through into case `2`, which breaks; case `3` falls through into `default`.

**Java-style (`->`) — automatic break:** each case ends implicitly, with no fall-through.

```
switch variable
    1 -> do_one()
    2 -> do_two()
    3 -> do_three()
    default -> do_default()
```

## Arithmetic operators

Same as in C:

+, -, *, /, %, ==, !=, <, >, <=, >=, +=, ++, etc.

## Logical and bitwise operators

Same as in C:

&&, ||, !, &, |, ^, ~, <<, >>

## Preprocessing directives

`#define A 3` stays the same, but `#include` directives change:
`#include stdio` -> `#include <stdio.h>`

## Structs, enums, and unions

Same as in C, but using indentation instead of `{}`.

## Pointers and arrays

`arr:int[10]` -> `int arr[10]`

Array literal elements are space-separated:

```
arr:int[3] = {1 2 3}
empty:int[10] = {}
```

Commas are not used inside array literals.

## Variables and constants

Same as in C.

## Literals

Same as in C, with one difference: octal literals use the `0o` prefix (Python-style) instead of C's bare `0` prefix.

- Decimal: `42`
- Hexadecimal: `0xFF`
- Octal: `0o77`
- Float: `3.14`, `1.0e-3`
- Character: `'a'`, `'\n'`
- String: `"hello"`
- Null: `null`

## Comments

We support both multi-line `/* */` and single-line `//` comments.

## Entry point

The `main` function is the entry point and is translated as:

`main` -> `int main(int argc, char * argv[])`

## Inline C

We allow embedding C code in blocks like this:

```text
`
some c code
`
```

## Type inference

Where possible, types are inferred (Stage 3 / code generation):

`x = 5` -> `int x = 5`
