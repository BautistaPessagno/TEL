#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AstBuilder.h"
#include "BisonParser.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

bool IsKnownTypedefName(const char * name);
void RegisterTypedefName(const char * name);
const char * BisonSourceName(void);
void SetBisonSourceName(const char * sourceName);
StatementList * FunctionBodyStatementListSemanticAction(StatementList * statementList);
Program * ProgramSemanticAction(ProgramItemList * items);

#endif
