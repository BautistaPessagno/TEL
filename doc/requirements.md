# Requirements

## Keywords

single words will be kept the same since they are just one token

- int -> int
- char -> char
- float -> float
- double -> double
- long -> long
- void -> void
- uint -> unsigned int
- uli -> unsigned long int

and they will be used like this

`x:int` -> `int x`

## Function declarations

we will use `fn name args -> return_type`

we can omit the return_type if the function returns void

### Example

instead of

```c
int foo(int arg1, char arg2);
```

we will have

```
fn foo arg1:int arg2:char
```

## Sentence termination

We will not be requiring `;` at the end of each line 

we will still be accepting programs that have it and ones that use it to separate sentences in the same line

## Indentation

Instead of using `{}` for blocks of code, we will be using indentation like python

## Flux control structures

### if-else

- `if` -> `if`
- `else if` -> `elif`
- `else` -> `else`

### loops

- `ford i 0 n` -> `for (int i = 0; i < n; i++)`
- `for i = 0, condition, i--` -> `for (int i = 0; condition i++)`
- `while` -> `while`
- `dw` -> `do while`

### Switch

the switch will have this structure

```c
switch variable
    1: ...
    2: ...
    default break
```

or if we want it to include a break in each case we can do like in java and use

```c
switch variable
    1 -> ...
    2 -> ...
    default break
```

## Arithmetic operators

They will be the same as in c

+, -, *, /, %, ==, !=, <, >, <=, >=, +=, ++, etc

## Logic Operators and bitwise operators

The same as in c

&&, ||, !, &, |, ^, ~, <<, >>

## Preprocessing directives

`#define A 3` stays the same, but when it comes to includes
`#include stdio` -> `#include <stdio.h>`

## Structs, enums, unions

they stay the same, but using indentation instead of `{}`

## Pointers and arrays

`arr:int[10]` ->  `int arr[10]`

## Variables and constants

they will stay the same

## Literals

they will stay the same

## Comments

we will support both multi line and //

## Entry Point

the main function will be the entry point where

`main` -> `int main(int argc, char * argv[])`

## implicit i/o

if the program uses i/o functions, stdio will automatically be included

## c blocks

we will allaw for c code in blocks like this

```c
\`
// some c code
\`
```

## Type inference

if we can we would like to include

`x = 5` -> `int x = 5`