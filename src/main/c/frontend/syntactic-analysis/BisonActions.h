#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions.
 */

Type * TypeSemanticAction(TypeKind kind);
Type * NamedTypeSemanticAction(TypeKind kind, char * name);
bool IsKnownTypedefName(const char * name);
VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type, Expression * initializer);
VariableDeclarationList * SingletonVariableDeclarationListSemanticAction(VariableDeclaration * declaration);
VariableDeclarationList * AppendVariableDeclarationListSemanticAction(VariableDeclarationList * declarationList, VariableDeclaration * declaration);
Parameter * ParameterSemanticAction(char * name, Type * type);
ParameterList * SingletonParameterListSemanticAction(Parameter * parameter);
ParameterList * AppendParameterListSemanticAction(ParameterList * parameterList, Parameter * parameter);
FunctionDeclaration * FunctionDeclarationSemanticAction(char * name, ParameterList * parameters, Type * returnType, TopLevelItemList * body);
AggregateDeclaration * AggregateDeclarationSemanticAction(AggregateKind kind, char * name, VariableDeclarationList * fields);
EnumMember * EnumMemberSemanticAction(char * name, char * value);
EnumMemberList * SingletonEnumMemberListSemanticAction(EnumMember * member);
EnumMemberList * AppendEnumMemberListSemanticAction(EnumMemberList * memberList, EnumMember * member);
EnumDeclaration * EnumDeclarationSemanticAction(char * name, EnumMemberList * members);
TypedefDeclaration * TypedefDeclarationSemanticAction(char * name, Type * type);
PreprocessorDirective * PreprocessorDirectiveSemanticAction(PreprocessorDirectiveKind kind, char * value);
FunctionCall * FunctionCallSemanticAction(char * name, ExpressionList * arguments);
Expression * IdentifierExpressionSemanticAction(char * value);
Expression * IntegerLiteralExpressionSemanticAction(char * value);
Expression * StringLiteralExpressionSemanticAction(char * value);
Expression * FunctionCallExpressionSemanticAction(FunctionCall * functionCall);
Expression * BinaryExpressionSemanticAction(Expression * left, ExpressionOperator operator, Expression * right);
Expression * UnaryExpressionSemanticAction(ExpressionOperator operator, Expression * operand);
ExpressionList * SingletonExpressionListSemanticAction(Expression * expression);
ExpressionList * AppendExpressionListSemanticAction(ExpressionList * expressionList, Expression * expression);
TopLevelItem * VariableDeclarationTopLevelItemSemanticAction(VariableDeclaration * declaration);
TopLevelItem * FunctionDeclarationTopLevelItemSemanticAction(FunctionDeclaration * declaration);
TopLevelItem * AggregateDeclarationTopLevelItemSemanticAction(AggregateDeclaration * declaration);
TopLevelItem * EnumDeclarationTopLevelItemSemanticAction(EnumDeclaration * declaration);
TopLevelItem * TypedefDeclarationTopLevelItemSemanticAction(TypedefDeclaration * declaration);
TopLevelItem * PreprocessorDirectiveTopLevelItemSemanticAction(PreprocessorDirective * directive);
TopLevelItem * FunctionCallTopLevelItemSemanticAction(FunctionCall * functionCall);
TopLevelItem * ReturnStatementTopLevelItemSemanticAction(Expression * expression);
TopLevelItem * ExpressionStatementTopLevelItemSemanticAction(Expression * expression);
TopLevelItem * EmptyStatementTopLevelItemSemanticAction();
TopLevelItemList * SingletonTopLevelItemListSemanticAction(TopLevelItem * item);
TopLevelItemList * AppendTopLevelItemListSemanticAction(TopLevelItemList * itemList, TopLevelItem * item);
TopLevelItemList * FunctionBodyTopLevelItemListSemanticAction(TopLevelItemList * itemList);
Program * ProgramSemanticAction(TopLevelItemList * items);

#endif
