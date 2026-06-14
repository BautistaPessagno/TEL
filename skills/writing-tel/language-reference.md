# TEL Language Reference

Complete syntax for writing TEL. TEL is indentation-based (Python-like) and compiles to C. Read this while writing a program; everyday rules live in `SKILL.md`.

All examples below are taken from the compiler's accepted test programs.

## Keywords

| Keyword | Meaning |
|---------|---------|
| `fn` | function declaration/definition |
| `ret` | return (`ret expr` or bare `ret` for void) |
| `main` | program entry point (no signature) |
| `if` `elif` `else` | conditionals (note `elif`, not `else if`) |
| `ford` | counting for loop |
| `for` | general for loop |
| `while` | while loop |
| `dw` | do-while loop |
| `switch` `default` `break` | switch statement |
| `cnt` | continue |
| `struct` `union` `enum` | aggregate types |
| `typedef` | type alias |
| `const` | const qualifier |
| `stc` | static storage (NOT `static`) |
| `null` | null pointer literal |

## Types

- **Primitives:** `int`, `char`, `float`, `double`, `void`, `long`, `short`, `uint` (→ `unsigned int`), `uli` (→ `unsigned long int`).
- **Pointer:** postfix `*` — `int*`, `char*`, `struct Node*`.
- **Array:** `int[10]` (sized) or `int[]` (unsized — **only valid as a function parameter type**).
- **Function pointer type:** `(fn* arg1 arg2 -> ret)`, e.g. `(fn* int int -> int)`, `(fn* int -> int)`, `(fn*)` (no args, void).
- **Named:** `struct Name`, `union Name`, `enum Name`, or a typedef name.
- **Qualifier:** `const` precedes the base type — `const int`, `const char*`.

## Declarations

Form: `name:type` with optional `= initializer`.

```
x:int
y:int = 42
ptr:int* = null
arr:int[3] = {1 2 3}
empty:int[10] = {}
const value:int = 100
stc counter:int = 0
```

Statements end at a newline; `;` is optional, and is required only to put multiple statements on one line:

```
add(value, 2); log("done"); add(1, 3)
```

## Functions

Definition — parameters are space-separated `name:type` pairs, return type after `->` (omit `->` for void):

```
fn add left:int right:int -> int
    ret left + right
```

Prototype (no body) — end with `;`:

```
fn log text:char* -> void;
fn add left:int right:int -> int;
```

`stc fn name ...` makes the function static. Functions may be called before they are defined (forward calls allowed).

### main

No signature; must have an indented body. Compiles to `int main(int argc, char* argv[])`.

```
main
    ret 0
```

### Returns and implicit returns

- Explicit: `ret expr`, or bare `ret` in a void function.
- **Implicit return:** the *final* statement of a function body returns automatically when it is:
  - a bare identifier — `total`
  - a comparison/logical expression — `a > b`, `x == y`, `!flag`, `result != -1 && result == 1`
- A **function-call** result is NOT returned implicitly — use `ret f(x)` explicitly.

```
fn id v:int -> int
    v                       // implicit: return v;
fn cmp a:int b:int -> int
    a > b                   // implicit: return (a > b);
fn sumTo n:int -> int
    total:int = 0
    ford i 0 n
        total += i
    ret total               // explicit
```

## Control flow

### if / elif / else

```
if x == 0
    x = 1
elif x == 1
    x = 2
else
    x = 3
```

### ford (counting loop)

`ford var start end` → `for (int var = start; var < end; var++)`. The iterator is an implicit local `int`. Start/end may be identifiers, int literals, or parenthesized expressions.

```
ford i 0 n
    total += i
ford j (i + 1) size
    ...
```

### for (general loop)

`for init, condition, increment` — three **comma-separated** parts:

```
for i = 0, i < n, i++
    n += 1
```

### while

```
while n > 0
    n--
```

### dw (do-while)

Body first; the condition appears on its own line after the body dedents:

```
dw
    n++
    n += 1
(n < 3)
```

### switch

Two case styles, mixable in one switch:
- `value:` → C-style block, falls through unless you `break`.
- `value ->` → auto-break (no fall-through). Body inline or indented.
- `default:` / `default ->` with a body, or `default break` on one line.

```
switch x
    1:
        x = 10
        break
    2 -> x = 20
    3 ->
        x = 30
    4: x = 40; break
    default break
```

### break / cnt

`break` exits a loop or switch. `cnt` is continue (NOT `continue`). Both valid only inside the matching context.

## Operators

| Class | Operators |
|-------|-----------|
| Arithmetic | `+ - * / %` |
| Relational | `< > <= >=` |
| Equality | `== !=` |
| Logical | `&& \|\| !` (NOT `and`/`or`/`not`) |
| Bitwise | `& \| ^ ~ << >>` |
| Assignment | `=  += -= *= /= %=  &= \|= ^= <<= >>=` |
| Unary | `+ - * & ! ~ ++ --` |
| Postfix | `[]` index, `.` member, `->` pointer member, `f(a, b)` call |

Precedence (high→low): postfix → prefix unary → `* / %` → `+ -` → `<< >>` → `< > <= >=` → `== !=` → `&` → `^` → `|` → `&&` → `||` → assignment (right-assoc). Same as C.

Member/pointer access:

```
a:int = p.x
b:int = ptr->x
c:int = points[0].y
d:int = node->next->value
e:int = getPoint()->x
ptr->y += b
```

## Literals

- **Integer:** decimal `42`; hex `0xFF` / `0X1A`; octal `0o17` / `0O7` (Python-style, NOT C `017`).
- **Float:** `3.14`, `.5`, `5.`, scientific `1e10`, `1E+3`, `1e-3`.
- **Char:** `'a'`, escapes `'\n' '\t' '\\' '\'' '\"' '\x41'`.
- **String:** `"hello"`, `"with\nescape"`, `"\"quoted\""`.
- **Array:** space-separated, `{1 2 3}`, empty `{}` (NOT comma-separated).
- **Null:** `null` (compatible with any object/function pointer).

## Aggregates and typedefs

```
struct Point
    x:int
    y:int

union Number
    asInt:int
    asDouble:double

enum Color
    RED
    GREEN = 2
    BLUE            // = 3 by default (next after 2)

typedef Count:uint
typedef PointAlias:struct Point
typedef Comparator:(fn* int int -> int)

origin:PointAlias
```

To use an aggregate as a type you keep the keyword: `b:struct Box`, `n:union Number`, `c:enum Color` (NOT bare `b:Box`). `typedef` an alias if you want the short name. Struct/union field names must be unique. Self-referential structs use a pointer: `next:struct Node*`.

## Preprocessor, comments, inline C

- `#include stdio` → `#include <stdio.h>` (the identifier becomes `<name.h>`).
- `#define A 3`, `#define SQUARE(x) ((x) * (x))` — copied through.
- Comments: `// line` and `/* block */`.
- **Inline C block** — raw C between lines that contain only a backtick. Copied verbatim, not type-checked. Use ONLY as an escape hatch (e.g. calling `printf`); writing logic here wastes the tokens TEL exists to save.

```
`
#include <stdio.h>
`

main
    `
    printf("hello, tel\n");
    `
    ret 0
```

## Indentation

- Blocks are defined by indentation, no braces. A tab = 4 spaces; indent in multiples of 4.
- Indentation increase opens a block; returning to the previous level closes it.
- Blank lines and comment-only lines are ignored for indentation.

## Key restrictions

1. Function-call arguments are **comma-separated**: `add(x, 2)` (not `add(x 2)`).
2. Array literals are **space-separated**: `{1 2 3}` (not `{1, 2, 3}`).
3. Implicit return only for a final bare identifier or comparison/logical expression — call results need explicit `ret`.
4. Unsized arrays `int[]` only as a parameter type, not a variable.
5. `const int*` may be assigned from `int*`, not vice versa.
6. Numeric types (`int`/`float`/`char`/`uint`/`uli`/`long`/`short`/enum) are mutually convertible.
7. At most one `main` per file (omit it for a library).
8. Identifiers may start with `_` (e.g. `_name`).

## Complete example programs (verbatim from accepted tests)

### Functions, prototypes, calls

```
value:int;
message:char
fn add left:int right:int -> int;
fn log text:char* -> void
main
    add(value, 2); log("done"); add(1, 3)
```

### Functions with implicit returns + control flow

```
fn matchesPair nums:int[] left:int right:int target:int -> int
    nums[left] + nums[right] == target

fn foundExpectedPair result:int -> int
    result != -1 && result == 1

fn twoSum nums:int[] size:int target:int -> int
    ford i 0 size
        ford j (i + 1) size
            if matchesPair(nums, i, j, target)
                ret i * 10 + j
    ret -1

main
    values:int[4] = {2 7 11 15}
    result:int = twoSum(values, 4, 9)
    if foundExpectedPair(result)
        ret result
    ret -1
```

### Arrays, pointers, inline C

```
`
#include <stdio.h>
`

fn sumArray data:int[] size:int -> int
    total:int = 0
    ford i 0 size
        total += data[i]
    ret total

main
    nums:int[5] = {1 2 3 4 5}
    s:int = sumArray(nums, 5)
    p:int*
    p = &s
    `
    printf("sum=%d\n", *p);
    `
    ret 0
```

### Structs and member access

```
struct Point
    x:int
    y:int

struct Node
    value:int
    next:struct Node*

fn getPoint -> struct Point*
    ret null

fn read p:struct Point ptr:struct Point* points:struct Point[] node:struct Node* -> int
    a:int = p.x
    b:int = ptr->x
    c:int = points[0].y
    d:int = node->next->value
    e:int = getPoint()->x
    p.x = a
    ptr->y += b
    ret c
```

### Static storage

```
stc counter:int = 0
stc fn tick -> int
    counter = counter + 1
    counter
fn use
    stc local:int = 7
    ret
```
