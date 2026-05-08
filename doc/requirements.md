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

## Statement termination

Semicolons are optional at the end of a line. To place multiple statements on the same line, separate them with `;`.

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

### Implicit I/O

If the program uses I/O functions, `stdio` is automatically included.

## Structs, enums, and unions

Same as in C, but using indentation instead of `{}`.

## Pointers and arrays

`arr:int[10]` -> `int arr[10]`

## Variables and constants

Same as in C.

## Literals

Same as in C.

## Comments

We support both multi-line `/* */` and single-line `//` comments.

## Entry point

The `main` function is the entry point and is translated as:

`main` -> `int main(int argc, char * argv[])`

## Inline C

We allow embedding C code in blocks like this:

```text
\`
// some c code
\`
```

## Type inference

Where possible, types are inferred:

`x = 5` -> `int x = 5`