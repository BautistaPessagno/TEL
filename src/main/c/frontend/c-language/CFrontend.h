#ifndef C_FRONTEND_HEADER
#define C_FRONTEND_HEADER

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/CompilerState.h"
#include <stdbool.h>
#include <stdio.h>

typedef struct CIdentifierBinding CIdentifierBinding;
typedef struct ParameterList ParameterList;

typedef struct {
	CIdentifierBinding * identifierBindings;
	CIdentifierBinding * pendingParameters;
	unsigned int scopeDepth;
} CFrontendContext;

bool isCFrontendTypedefName(CFrontendContext * context, const char * name);
void registerCFrontendTypedef(void * scanner, const char * name);
void registerCFrontendOrdinary(void * scanner, const char * name);
void prepareCFrontendFunctionParameters(void * scanner, ParameterList * parameters);
void pushCFrontendScope(CFrontendContext * context);
void popCFrontendScope(CFrontendContext * context);

CompilationStatus executeCFrontend(
	FILE * input,
	const char * sourceName,
	CompilerState * compilerState);

#endif
