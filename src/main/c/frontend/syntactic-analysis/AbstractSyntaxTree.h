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
typedef enum TopLevelItemKind TopLevelItemKind;
typedef enum ExpressionKind ExpressionKind;
typedef enum ExpressionOperator ExpressionOperator;

typedef struct Type Type;
typedef struct VariableDeclaration VariableDeclaration;
typedef struct Parameter Parameter;
typedef struct ParameterList ParameterList;
typedef struct FunctionDeclaration FunctionDeclaration;
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
	TYPE_ULI_KIND
};

enum TopLevelItemKind {
	TOP_LEVEL_VARIABLE_DECLARATION,
	TOP_LEVEL_FUNCTION_DECLARATION,
	TOP_LEVEL_FUNCTION_CALL,
	TOP_LEVEL_RETURN_STATEMENT,
	TOP_LEVEL_EXPRESSION_STATEMENT,
	TOP_LEVEL_EMPTY_STATEMENT
};

enum ExpressionKind {
	EXPRESSION_IDENTIFIER,
	EXPRESSION_INTEGER_LITERAL,
	EXPRESSION_STRING_LITERAL,
	EXPRESSION_FUNCTION_CALL,
	EXPRESSION_BINARY_OPERATION,
	EXPRESSION_UNARY_OPERATION
};

enum ExpressionOperator {
	EXPRESSION_OPERATOR_ASSIGN,
	EXPRESSION_OPERATOR_ADD_ASSIGN,
	EXPRESSION_OPERATOR_SUBTRACT_ASSIGN,
	EXPRESSION_OPERATOR_MULTIPLY_ASSIGN,
	EXPRESSION_OPERATOR_DIVIDE_ASSIGN,
	EXPRESSION_OPERATOR_MODULO_ASSIGN,
	EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN,
	EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN,
	EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN,
	EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN,
	EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN,
	EXPRESSION_OPERATOR_LOGICAL_OR,
	EXPRESSION_OPERATOR_LOGICAL_AND,
	EXPRESSION_OPERATOR_BITWISE_OR,
	EXPRESSION_OPERATOR_BITWISE_XOR,
	EXPRESSION_OPERATOR_BITWISE_AND,
	EXPRESSION_OPERATOR_EQUAL,
	EXPRESSION_OPERATOR_NOT_EQUAL,
	EXPRESSION_OPERATOR_LESS_THAN,
	EXPRESSION_OPERATOR_GREATER_THAN,
	EXPRESSION_OPERATOR_LESS_EQUAL,
	EXPRESSION_OPERATOR_GREATER_EQUAL,
	EXPRESSION_OPERATOR_SHIFT_LEFT,
	EXPRESSION_OPERATOR_SHIFT_RIGHT,
	EXPRESSION_OPERATOR_ADD,
	EXPRESSION_OPERATOR_SUBTRACT,
	EXPRESSION_OPERATOR_MULTIPLY,
	EXPRESSION_OPERATOR_DIVIDE,
	EXPRESSION_OPERATOR_MODULO,
	EXPRESSION_OPERATOR_UNARY_PLUS,
	EXPRESSION_OPERATOR_UNARY_MINUS,
	EXPRESSION_OPERATOR_LOGICAL_NOT,
	EXPRESSION_OPERATOR_BITWISE_NOT,
	EXPRESSION_OPERATOR_PREFIX_INCREMENT,
	EXPRESSION_OPERATOR_PREFIX_DECREMENT,
	EXPRESSION_OPERATOR_POSTFIX_INCREMENT,
	EXPRESSION_OPERATOR_POSTFIX_DECREMENT
};

struct Type {
	TypeKind kind;
};

struct VariableDeclaration {
	char * name;
	Type * type;
	Expression * initializer;
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

struct FunctionCall {
	char * name;
	ExpressionList * arguments;
};

struct Expression {
	ExpressionKind kind;
	char * value;
	FunctionCall * functionCall;
	ExpressionOperator operator;
	Expression * left;
	Expression * right;
};

struct ExpressionList {
	Expression * expression;
	ExpressionList * next;
};

struct TopLevelItem {
	TopLevelItemKind kind;
	VariableDeclaration * variableDeclaration;
	FunctionDeclaration * functionDeclaration;
	FunctionCall * functionCall;
	Expression * expression;
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
void destroyParameter(Parameter * parameter);
void destroyParameterList(ParameterList * parameterList);
void destroyFunctionDeclaration(FunctionDeclaration * functionDeclaration);
void destroyFunctionCall(FunctionCall * functionCall);
void destroyExpression(Expression * expression);
void destroyExpressionList(ExpressionList * expressionList);
void destroyTopLevelItem(TopLevelItem * topLevelItem);
void destroyTopLevelItemList(TopLevelItemList * topLevelItemList);
void destroyProgram(Program * program);

#endif
