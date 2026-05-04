#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */

typedef enum TypeKind TypeKind;
typedef enum AggregateKind AggregateKind;
typedef enum TopLevelItemKind TopLevelItemKind;
typedef enum ExpressionKind ExpressionKind;

typedef struct Type Type;
typedef struct VariableDeclaration VariableDeclaration;
typedef struct VariableDeclarationList VariableDeclarationList;
typedef struct Parameter Parameter;
typedef struct ParameterList ParameterList;
typedef struct FunctionDeclaration FunctionDeclaration;
typedef struct AggregateDeclaration AggregateDeclaration;
typedef struct EnumMember EnumMember;
typedef struct EnumMemberList EnumMemberList;
typedef struct EnumDeclaration EnumDeclaration;
typedef struct TypedefDeclaration TypedefDeclaration;
typedef struct FunctionCall FunctionCall;
typedef struct Expression Expression;
typedef struct ExpressionList ExpressionList;
typedef struct TopLevelItem TopLevelItem;
typedef struct TopLevelItemList TopLevelItemList;
typedef struct Program Program;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */

enum TypeKind {
	TYPE_INT_KIND,
	TYPE_CHAR_KIND,
	TYPE_FLOAT_KIND,
	TYPE_DOUBLE_KIND,
	TYPE_VOID_KIND,
	TYPE_UINT_KIND,
	TYPE_ULI_KIND,
	TYPE_LONG_KIND,
	TYPE_NAMED_KIND,
	TYPE_STRUCT_KIND,
	TYPE_ENUM_KIND,
	TYPE_UNION_KIND
};

enum AggregateKind {
	AGGREGATE_STRUCT_KIND,
	AGGREGATE_UNION_KIND
};

enum TopLevelItemKind {
	TOP_LEVEL_VARIABLE_DECLARATION,
	TOP_LEVEL_FUNCTION_DECLARATION,
	TOP_LEVEL_AGGREGATE_DECLARATION,
	TOP_LEVEL_ENUM_DECLARATION,
	TOP_LEVEL_TYPEDEF_DECLARATION,
	TOP_LEVEL_FUNCTION_CALL,
	TOP_LEVEL_EMPTY_STATEMENT
};

enum ExpressionKind {
	EXPRESSION_IDENTIFIER,
	EXPRESSION_INTEGER_LITERAL,
	EXPRESSION_STRING_LITERAL,
	EXPRESSION_FUNCTION_CALL
};

struct Type {
	TypeKind kind;
	char * name;
};

struct VariableDeclaration {
	char * name;
	Type * type;
};

struct VariableDeclarationList {
	VariableDeclaration * declaration;
	VariableDeclarationList * next;
};

struct Parameter {
	char * name;
	Type * type;
};

struct ParameterList {
	Parameter * parameter;
	ParameterList * next;
};

struct FunctionDeclaration {
	char * name;
	ParameterList * parameters;
	Type * returnType;
	TopLevelItemList * body;
};

struct AggregateDeclaration {
	AggregateKind kind;
	char * name;
	VariableDeclarationList * fields;
};

struct EnumMember {
	char * name;
	char * value;
};

struct EnumMemberList {
	EnumMember * member;
	EnumMemberList * next;
};

struct EnumDeclaration {
	char * name;
	EnumMemberList * members;
};

struct TypedefDeclaration {
	char * name;
	Type * type;
};

struct FunctionCall {
	char * name;
	ExpressionList * arguments;
};

struct Expression {
	ExpressionKind kind;
	char * value;
	FunctionCall * functionCall;
};

struct ExpressionList {
	Expression * expression;
	ExpressionList * next;
};

struct TopLevelItem {
	TopLevelItemKind kind;
	VariableDeclaration * variableDeclaration;
	FunctionDeclaration * functionDeclaration;
	AggregateDeclaration * aggregateDeclaration;
	EnumDeclaration * enumDeclaration;
	TypedefDeclaration * typedefDeclaration;
	FunctionCall * functionCall;
};

struct TopLevelItemList {
	TopLevelItem * item;
	TopLevelItemList * next;
};

struct Program {
	TopLevelItemList * items;
};

/**
 * Node recursive super-duper-trambolik-destructors.
 */

void destroyType(Type * type);
void destroyVariableDeclaration(VariableDeclaration * variableDeclaration);
void destroyVariableDeclarationList(VariableDeclarationList * variableDeclarationList);
void destroyParameter(Parameter * parameter);
void destroyParameterList(ParameterList * parameterList);
void destroyFunctionDeclaration(FunctionDeclaration * functionDeclaration);
void destroyAggregateDeclaration(AggregateDeclaration * aggregateDeclaration);
void destroyEnumMember(EnumMember * enumMember);
void destroyEnumMemberList(EnumMemberList * enumMemberList);
void destroyEnumDeclaration(EnumDeclaration * enumDeclaration);
void destroyTypedefDeclaration(TypedefDeclaration * typedefDeclaration);
void destroyFunctionCall(FunctionCall * functionCall);
void destroyExpression(Expression * expression);
void destroyExpressionList(ExpressionList * expressionList);
void destroyTopLevelItem(TopLevelItem * topLevelItem);
void destroyTopLevelItemList(TopLevelItemList * topLevelItemList);
void destroyProgram(Program * program);

#endif
