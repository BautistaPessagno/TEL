#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions.
 */

Type * IntTypeSemanticAction();
VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type);
VariableDeclarationList * SingletonDeclarationListSemanticAction(VariableDeclaration * declaration);
VariableDeclarationList * AppendDeclarationListSemanticAction(VariableDeclarationList * declarationList, VariableDeclaration * declaration);
Program * ProgramSemanticAction(VariableDeclarationList * declarations);

#endif
