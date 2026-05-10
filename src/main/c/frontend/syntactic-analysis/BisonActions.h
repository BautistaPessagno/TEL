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
Type * PointerTypeSemanticAction(Type * pointee);
Type * ArrayTypeSemanticAction(Type * element, Expression * size);
Type * FunctionPointerTypeSemanticAction(ParameterList * params, Type * returnType);
ParameterList * SingletonBareParameterListSemanticAction(Type * type);
ParameterList * AppendBareParameterListSemanticAction(ParameterList * list, Type * type);
bool IsKnownTypedefName(const char * name);
VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type, Expression * initializer);
VariableDeclarationList * SingletonVariableDeclarationListSemanticAction(VariableDeclaration * declaration);
VariableDeclarationList * AppendVariableDeclarationListSemanticAction(VariableDeclarationList * declarationList, VariableDeclaration * declaration);
Parameter * ParameterSemanticAction(char * name, Type * type);
ParameterList * SingletonParameterListSemanticAction(Parameter * parameter);
ParameterList * AppendParameterListSemanticAction(ParameterList * parameterList, Parameter * parameter);
FunctionDeclaration * FunctionDeclarationSemanticAction(char * name, ParameterList * parameters, Type * returnType, StatementList * body);
MainDeclaration * MainDeclarationSemanticAction(StatementList * body);
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
Expression * FloatLiteralExpressionSemanticAction(char * value);
Expression * CharLiteralExpressionSemanticAction(char * value);
Expression * StringLiteralExpressionSemanticAction(char * value);
Expression * NullLiteralExpressionSemanticAction(void);
Expression * ArrayLiteralExpressionSemanticAction(ExpressionList * elements);
Expression * FunctionCallExpressionSemanticAction(FunctionCall * functionCall);
Expression * BinaryExpressionSemanticAction(Expression * left, ExpressionOperator operator, Expression * right);
Expression * UnaryExpressionSemanticAction(ExpressionOperator operator, Expression * operand);
ExpressionList * SingletonExpressionListSemanticAction(Expression * expression);
ExpressionList * AppendExpressionListSemanticAction(ExpressionList * expressionList, Expression * expression);
ProgramItem * VariableDeclarationProgramItemSemanticAction(VariableDeclaration * declaration);
ProgramItem * FunctionDeclarationProgramItemSemanticAction(FunctionDeclaration * declaration);
ProgramItem * MainDeclarationProgramItemSemanticAction(MainDeclaration * declaration);
ProgramItem * AggregateDeclarationProgramItemSemanticAction(AggregateDeclaration * declaration);
ProgramItem * EnumDeclarationProgramItemSemanticAction(EnumDeclaration * declaration);
ProgramItem * TypedefDeclarationProgramItemSemanticAction(TypedefDeclaration * declaration);
ProgramItem * PreprocessorDirectiveProgramItemSemanticAction(PreprocessorDirective * directive);
ProgramItem * EmptyProgramItemSemanticAction();
ProgramItemList * SingletonProgramItemListSemanticAction(ProgramItem * item);
ProgramItemList * AppendProgramItemListSemanticAction(ProgramItemList * itemList, ProgramItem * item);
ProgramItemList * ConcatenateProgramItemListSemanticAction(ProgramItemList * first, ProgramItemList * second);
Statement * VariableDeclarationStatementSemanticAction(VariableDeclaration * declaration);
Statement * ReturnStatementSemanticAction(Expression * expression);
Statement * ExpressionStatementSemanticAction(Expression * expression);
IfBranch * IfBranchSemanticAction(Expression * condition, StatementList * body);
IfBranch * AppendIfBranchSemanticAction(IfBranch * branchList, IfBranch * branch);
IfStatement * IfStatementSemanticAction(IfBranch * branches, StatementList * elseBody);
ForStatement * ForStatementSemanticAction(Expression * initializer, Expression * condition, Expression * update, StatementList * body);
ForStatement * ForStatementFromFordSemanticAction(char * iteratorName, Expression * start, Expression * end, StatementList * body);
WhileStatement * WhileStatementSemanticAction(Expression * condition, StatementList * body);
DoWhileStatement * DoWhileStatementSemanticAction(StatementList * body, Expression * condition);
SwitchCase * SwitchCaseSemanticAction(Expression * matchExpression, StatementList * body);
SwitchCase * AppendSwitchCaseSemanticAction(SwitchCase * caseList, SwitchCase * switchCase);
SwitchStatement * SwitchStatementSemanticAction(Expression * discriminant, SwitchCase * cases);
Statement * IfStatementSemanticActionWrapper(IfStatement * ifStatement);
Statement * ForStatementSemanticActionWrapper(ForStatement * forStatement);
Statement * WhileStatementSemanticActionWrapper(WhileStatement * whileStatement);
Statement * DoWhileStatementSemanticActionWrapper(DoWhileStatement * doWhileStatement);
Statement * SwitchStatementSemanticActionWrapper(SwitchStatement * switchStatement);
Statement * BreakStatementSemanticAction();
Statement * EmptyStatementSemanticAction();
StatementList * SingletonStatementListSemanticAction(Statement * statement);
StatementList * AppendStatementListSemanticAction(StatementList * statementList, Statement * statement);
StatementList * FunctionBodyStatementListSemanticAction(StatementList * statementList);
Program * ProgramSemanticAction(ProgramItemList * items);

#endif
