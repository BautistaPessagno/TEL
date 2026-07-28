#ifndef TEL_CODE_GENERATOR_HEADER
#define TEL_CODE_GENERATOR_HEADER

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"
#include <stdio.h>

CompilationStatus executeTelCodeGeneration(CompilerState * compilerState, FILE * output);

#endif
