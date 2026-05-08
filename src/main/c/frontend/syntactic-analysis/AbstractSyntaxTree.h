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
typedef enum PreprocessorDirectiveKind PreprocessorDirectiveKind;
typedef enum ProgramItemKind ProgramItemKind;
typedef enum StatementKind StatementKind;
typedef enum ExpressionKind ExpressionKind;
typedef enum ExpressionOperator ExpressionOperator;

typedef struct Type Type;
typedef struct VariableDeclaration VariableDeclaration;
typedef struct VariableDeclarationList VariableDeclarationList;
typedef struct Parameter Parameter;
typedef struct ParameterList ParameterList;
typedef struct FunctionDeclaration FunctionDeclaration;
typedef struct MainDeclaration MainDeclaration;
typedef struct AggregateDeclaration AggregateDeclaration;
typedef struct EnumMember EnumMember;
typedef struct EnumMemberList EnumMemberList;
typedef struct EnumDeclaration EnumDeclaration;
typedef struct TypedefDeclaration TypedefDeclaration;
typedef struct PreprocessorDirective PreprocessorDirective;
typedef struct FunctionCall FunctionCall;
typedef struct Expression Expression;
typedef struct ExpressionList ExpressionList;
typedef struct ProgramItem ProgramItem;
typedef struct ProgramItemList ProgramItemList;
typedef struct IfBranch IfBranch;
typedef struct IfStatement IfStatement;
typedef struct ForStatement ForStatement;
typedef struct WhileStatement WhileStatement;
typedef struct DoWhileStatement DoWhileStatement;
typedef struct SwitchCase SwitchCase;
typedef struct SwitchStatement SwitchStatement;
typedef struct Statement Statement;
typedef struct StatementList StatementList;
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
	TYPE_UNION_KIND,
	TYPE_POINTER_KIND,
	TYPE_ARRAY_KIND
};

enum AggregateKind {
	AGGREGATE_STRUCT_KIND,
	AGGREGATE_UNION_KIND
};

enum PreprocessorDirectiveKind {
	PREPROCESSOR_INCLUDE_DIRECTIVE,
	PREPROCESSOR_DEFINE_DIRECTIVE
};

enum ProgramItemKind {
	PROGRAM_ITEM_VARIABLE_DECLARATION,
	PROGRAM_ITEM_FUNCTION_DECLARATION,
	PROGRAM_ITEM_MAIN_DECLARATION,
	PROGRAM_ITEM_AGGREGATE_DECLARATION,
	PROGRAM_ITEM_ENUM_DECLARATION,
	PROGRAM_ITEM_TYPEDEF_DECLARATION,
	PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE,
	PROGRAM_ITEM_EMPTY
};

enum StatementKind {
	STATEMENT_VARIABLE_DECLARATION,
	STATEMENT_RETURN,
	STATEMENT_EXPRESSION,
	STATEMENT_IF,
	STATEMENT_FOR,
	STATEMENT_WHILE,
	STATEMENT_DO_WHILE,
	STATEMENT_SWITCH,
	STATEMENT_BREAK,
	STATEMENT_EMPTY
};

enum ExpressionKind {
	EXPRESSION_IDENTIFIER,
	EXPRESSION_INTEGER_LITERAL,
	EXPRESSION_STRING_LITERAL,
	EXPRESSION_FUNCTION_CALL,
	EXPRESSION_BINARY_OPERATION,
	EXPRESSION_UNARY_OPERATION,
	EXPRESSION_NULL_LITERAL
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
	EXPRESSION_OPERATOR_DEREFERENCE,
	EXPRESSION_OPERATOR_ADDRESS_OF,
	EXPRESSION_OPERATOR_ARRAY_INDEX,
	EXPRESSION_OPERATOR_PREFIX_INCREMENT,
	EXPRESSION_OPERATOR_PREFIX_DECREMENT,
	EXPRESSION_OPERATOR_POSTFIX_INCREMENT,
	EXPRESSION_OPERATOR_POSTFIX_DECREMENT
};

struct Type {
	TypeKind kind;
	char * name;
	Type * pointee;
	Expression * arraySize;
	ParameterList * functionParams;
	Type * returnType;
};

struct VariableDeclaration {
	char * name;
	Type * type;
	Expression * initializer;
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
	StatementList * body;
};

struct MainDeclaration {
	StatementList * body;
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

struct PreprocessorDirective {
	PreprocessorDirectiveKind kind;
	char * value;
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
	/* Unary operations use operand; binary operations use left and right. */
	Expression * operand;
	Expression * left;
	Expression * right;
};

struct ExpressionList {
	Expression * expression;
	ExpressionList * next;
};

struct ProgramItem {
	ProgramItemKind kind;
	VariableDeclaration * variableDeclaration;
	FunctionDeclaration * functionDeclaration;
	MainDeclaration * mainDeclaration;
	AggregateDeclaration * aggregateDeclaration;
	EnumDeclaration * enumDeclaration;
	TypedefDeclaration * typedefDeclaration;
	PreprocessorDirective * preprocessorDirective;
};

struct ProgramItemList {
	ProgramItem * item;
	ProgramItemList * next;
};

struct IfBranch {
	Expression * condition;
	StatementList * body;
	IfBranch * next;
};

struct IfStatement {
	IfBranch * branches;
	StatementList * elseBody;
};

struct ForStatement {
	Expression * initializer;
	Expression * condition;
	Expression * update;
	StatementList * body;
};

struct WhileStatement {
	Expression * condition;
	StatementList * body;
};

struct DoWhileStatement {
	StatementList * body;
	Expression * condition;
};

struct SwitchCase {
	Expression * matchExpression;
	StatementList * body;
	SwitchCase * next;
};

struct SwitchStatement {
	Expression * discriminant;
	SwitchCase * cases;
};

struct Statement {
	StatementKind kind;
	VariableDeclaration * variableDeclaration;
	Expression * expression;
	IfStatement * ifStatement;
	ForStatement * forStatement;
	WhileStatement * whileStatement;
	DoWhileStatement * doWhileStatement;
	SwitchStatement * switchStatement;
};

struct StatementList {
	Statement * statement;
	StatementList * next;
};

struct Program {
	ProgramItemList * items;
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
void destroyMainDeclaration(MainDeclaration * mainDeclaration);
void destroyAggregateDeclaration(AggregateDeclaration * aggregateDeclaration);
void destroyEnumMember(EnumMember * enumMember);
void destroyEnumMemberList(EnumMemberList * enumMemberList);
void destroyEnumDeclaration(EnumDeclaration * enumDeclaration);
void destroyTypedefDeclaration(TypedefDeclaration * typedefDeclaration);
void destroyPreprocessorDirective(PreprocessorDirective * preprocessorDirective);
void destroyFunctionCall(FunctionCall * functionCall);
void destroyExpression(Expression * expression);
void destroyExpressionList(ExpressionList * expressionList);
void destroyProgramItem(ProgramItem * programItem);
void destroyProgramItemList(ProgramItemList * programItemList);
void destroyIfBranch(IfBranch * ifBranch);
void destroyIfStatement(IfStatement * ifStatement);
void destroyForStatement(ForStatement * forStatement);
void destroyWhileStatement(WhileStatement * whileStatement);
void destroyDoWhileStatement(DoWhileStatement * doWhileStatement);
void destroySwitchCase(SwitchCase * switchCase);
void destroySwitchStatement(SwitchStatement * switchStatement);
void destroyStatement(Statement * statement);
void destroyStatementList(StatementList * statementList);
void destroyProgram(Program * program);

#endif
