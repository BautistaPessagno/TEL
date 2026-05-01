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

Type * TypeSemanticAction(TypeKind kind);
VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type);
Parameter * ParameterSemanticAction(char * name, Type * type);
ParameterList * SingletonParameterListSemanticAction(Parameter * parameter);
ParameterList * AppendParameterListSemanticAction(ParameterList * parameterList, Parameter * parameter);
FunctionDeclaration * FunctionDeclarationSemanticAction(char * name, ParameterList * parameters, Type * returnType);
FunctionCall * FunctionCallSemanticAction(char * name, ExpressionList * arguments);
Expression * IdentifierExpressionSemanticAction(char * value);
Expression * IntegerLiteralExpressionSemanticAction(char * value);
Expression * StringLiteralExpressionSemanticAction(char * value);
Expression * FunctionCallExpressionSemanticAction(FunctionCall * functionCall);
ExpressionList * SingletonExpressionListSemanticAction(Expression * expression);
ExpressionList * AppendExpressionListSemanticAction(ExpressionList * expressionList, Expression * expression);
TopLevelItem * VariableDeclarationTopLevelItemSemanticAction(VariableDeclaration * declaration);
TopLevelItem * FunctionDeclarationTopLevelItemSemanticAction(FunctionDeclaration * declaration);
TopLevelItem * FunctionCallTopLevelItemSemanticAction(FunctionCall * functionCall);
TopLevelItemList * SingletonTopLevelItemListSemanticAction(TopLevelItem * item);
TopLevelItemList * AppendTopLevelItemListSemanticAction(TopLevelItemList * itemList, TopLevelItem * item);
Program * ProgramSemanticAction(TopLevelItemList * items);

#endif
