#ifndef PROGRAM_NORMALIZER_HEADER
#define PROGRAM_NORMALIZER_HEADER

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"

/**
 * Canonicalize a semantically valid program before either backend emits it.
 *
 * Semantic analysis must run first so normalization cannot hide incompatible
 * redeclarations.
 */
CompilationStatus normalizeProgram(CompilerState * compilerState);

#endif
