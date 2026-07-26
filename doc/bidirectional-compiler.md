# Bidirectional TEL ↔ C Compiler

## Command contract

The compiler chooses the source language from a named input path:

```text
tel program.tel                 TEL → C on stdout
tel program.c                   C → TEL on stdout
tel program.tel -o program.c
tel program.c -o program.tel
```

Extensions are case-sensitive. Only `.tel` and `.c` are accepted. A named file
cannot be combined with `--from`, because its extension is authoritative.

Legacy stdin remains supported:

```text
tel                             TEL stdin → C stdout
tel --from tel                  TEL stdin → C stdout
tel --from c                    C stdin → TEL stdout
```

`-o PATH` is valid with named or stdin input. Output is generated in memory
only after parsing and semantic validation succeed. File output is written to a
temporary sibling and renamed on success, preserving an existing destination
on every failure. Diagnostics go to stderr and failures return nonzero.

## Canonical identity

The compiler is bijective over canonical syntax/AST identity:

```text
CToTEL(TELToC(t)) = canonicalTEL(t)
TELToC(CToTEL(c)) = canonicalC(c)
```

Canonicalization occurs after semantic validation and:

- removes empty nodes;
- removes prototypes matched by definitions and collapses repeated compatible
  prototype-only declarations;
- orders directives, inline C, enums, typedefs, aggregates, prototypes,
  globals, functions, and `main` stably;
- normalizes decimal leading zeros and TEL octal prefixes;
- emits one forward prototype per defined function;
- emits deterministic whitespace and precedence-preserving expressions.

Ordinary comments, source formatting, and redundant parentheses are outside
canonical identity. Handwritten supported C may normalize to a different but
stable C spelling.

## Supported C subset

The C frontend accepts the constructs represented by the shared TEL AST:

- `int`, `char`, `float`, `double`, `void`, `short`, `long`, `unsigned int`,
  `unsigned long int`, base `const`, and `static`;
- named `struct`, `union`, and `enum` types, typedef names, pointers, arrays,
  function pointers, and compound declarators;
- globals, locals, comma declarations, prototypes, definitions, and common
  `int main(void)` / `int main(int argc, char **argv|*argv[])` definitions;
- named aggregate definitions, enums, typedefs, nested initializer lists, and
  function-pointer typedefs;
- returns, expression statements, braced or single-statement conditionals,
  `else if`, general `for`, canonical declared counting loops, `while`,
  `do/while`, `switch`, `break`, and `continue`;
- the shared expression/operator set, function calls, member/index access,
  hexadecimal/decimal/C-octal integers, floats, chars, strings, `NULL`, and the
  compiler-generated null expression;
- simple `<name.h>` system includes and opaque `#define` bodies;
- ordinary comments, which are discarded.

C octal literals become TEL `0o`; null expressions become `null`; canonical
counting loops become `ford`; trailing switch-case `break` becomes TEL `->`
when representable. Prototype parameters without C names receive deterministic
TEL names (`_p0`, `_p1`, ...). Typedef spellings remain usable in C's separate
field namespace and may be shadowed by scoped parameter or local names.

## Unsupported C

The frontend rejects syntax that cannot be represented by the shared AST,
including:

- general casts, `sizeof`, ternary and comma expressions;
- variadics, bitfields, designated initializers, anonymous aggregates;
- unsupported qualifiers/type combinations;
- labels and `goto`;
- conditional preprocessing and quoted/path includes;
- noncanonical declared `for` initializers;
- `main` prototypes, renamed argument parameters, and non-common `main`
  signatures.

Macros are preserved but not expanded. A macro name used in a typed expression
therefore remains outside the semantic subset.

## Inline C

Generated C wraps every TEL inline-C block in reserved valid-C comments:

```c
/*__TEL_INLINE_C_BEGIN__*/
/* raw payload */
/*__TEL_INLINE_C_END__*/
```

The C scanner reconstructs these blocks as opaque global or statement AST
nodes, normalizes line endings, and keeps one trailing newline. Ordinary C
comments are discarded. Payloads containing either reserved marker identifier
are rejected.

## Verification

`src/main/bash/test.sh` verifies TEL acceptance/rejection, CLI dispatch and
atomic output, TEL-to-C and C-to-TEL goldens, unsupported C diagnostics,
generated-C syntax, runnable programs, and both canonical inverse laws under
the AddressSanitizer build.
