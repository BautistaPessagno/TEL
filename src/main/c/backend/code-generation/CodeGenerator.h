#ifndef CODE_GENERATOR_HEADER
#define CODE_GENERATOR_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeCodeGeneratorModule();

/**
 * Generates the final C output using the current compiler state.
 */
CompilationStatus executeCodeGeneration(CompilerState * compilerState);

#endif
