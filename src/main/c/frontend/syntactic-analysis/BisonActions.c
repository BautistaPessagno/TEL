#include "BisonActions.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

typedef struct TypedefNameNode TypedefNameNode;

struct TypedefNameNode {
	char * name;
	TypedefNameNode * next;
};

static TypedefNameNode * _typedefNames = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	while (_typedefNames != NULL) {
		TypedefNameNode * next = _typedefNames->next;
		free(_typedefNames->name);
		free(_typedefNames);
		_typedefNames = next;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* IMPORTED FUNCTIONS */

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);
static void _registerTypedefName(const char * name);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

static void _registerTypedefName(const char * name) {
	if (IsKnownTypedefName(name)) {
		return;
	}
	TypedefNameNode * node = calloc(1, sizeof(TypedefNameNode));
	node->name = strdup(name);
	node->next = _typedefNames;
	_typedefNames = node;
}

/* PUBLIC FUNCTIONS */

Type * TypeSemanticAction(TypeKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = kind;
	return type;
}

Type * NamedTypeSemanticAction(TypeKind kind, char * name) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = TypeSemanticAction(kind);
	type->name = name;
	return type;
}

bool IsKnownTypedefName(const char * name) {
	for (TypedefNameNode * node = _typedefNames; node != NULL; node = node->next) {
		if (strcmp(node->name, name) == 0) {
			return true;
		}
	}
	return false;
}

VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclaration * declaration = calloc(1, sizeof(VariableDeclaration));
	declaration->name = name;
	declaration->type = type;
	return declaration;
}

VariableDeclarationList * SingletonVariableDeclarationListSemanticAction(VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclarationList * declarationList = calloc(1, sizeof(VariableDeclarationList));
	declarationList->declaration = declaration;
	return declarationList;
}

VariableDeclarationList * AppendVariableDeclarationListSemanticAction(VariableDeclarationList * declarationList, VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclarationList * tail = declarationList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonVariableDeclarationListSemanticAction(declaration);
	return declarationList;
}

Parameter * ParameterSemanticAction(char * name, Type * type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Parameter * parameter = calloc(1, sizeof(Parameter));
	parameter->name = name;
	parameter->type = type;
	return parameter;
}

ParameterList * SingletonParameterListSemanticAction(Parameter * parameter) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ParameterList * parameterList = calloc(1, sizeof(ParameterList));
	parameterList->parameter = parameter;
	return parameterList;
}

ParameterList * AppendParameterListSemanticAction(ParameterList * parameterList, Parameter * parameter) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ParameterList * tail = parameterList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonParameterListSemanticAction(parameter);
	return parameterList;
}

FunctionDeclaration * FunctionDeclarationSemanticAction(char * name, ParameterList * parameters, Type * returnType, TopLevelItemList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	FunctionDeclaration * declaration = calloc(1, sizeof(FunctionDeclaration));
	declaration->name = name;
	declaration->parameters = parameters;
	declaration->returnType = returnType;
	declaration->body = body;
	return declaration;
}

AggregateDeclaration * AggregateDeclarationSemanticAction(AggregateKind kind, char * name, VariableDeclarationList * fields) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	AggregateDeclaration * declaration = calloc(1, sizeof(AggregateDeclaration));
	declaration->kind = kind;
	declaration->name = name;
	declaration->fields = fields;
	return declaration;
}

EnumMember * EnumMemberSemanticAction(char * name, char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EnumMember * member = calloc(1, sizeof(EnumMember));
	member->name = name;
	member->value = value;
	return member;
}

EnumMemberList * SingletonEnumMemberListSemanticAction(EnumMember * member) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EnumMemberList * memberList = calloc(1, sizeof(EnumMemberList));
	memberList->member = member;
	return memberList;
}

EnumMemberList * AppendEnumMemberListSemanticAction(EnumMemberList * memberList, EnumMember * member) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EnumMemberList * tail = memberList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonEnumMemberListSemanticAction(member);
	return memberList;
}

EnumDeclaration * EnumDeclarationSemanticAction(char * name, EnumMemberList * members) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	EnumDeclaration * declaration = calloc(1, sizeof(EnumDeclaration));
	declaration->name = name;
	declaration->members = members;
	return declaration;
}

TypedefDeclaration * TypedefDeclarationSemanticAction(char * name, Type * type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TypedefDeclaration * declaration = calloc(1, sizeof(TypedefDeclaration));
	declaration->name = name;
	declaration->type = type;
	_registerTypedefName(name);
	return declaration;
}

FunctionCall * FunctionCallSemanticAction(char * name, ExpressionList * arguments) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	FunctionCall * functionCall = calloc(1, sizeof(FunctionCall));
	functionCall->name = name;
	functionCall->arguments = arguments;
	return functionCall;
}

Expression * IdentifierExpressionSemanticAction(char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_IDENTIFIER;
	expression->value = value;
	return expression;
}

Expression * IntegerLiteralExpressionSemanticAction(char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_INTEGER_LITERAL;
	expression->value = value;
	return expression;
}

Expression * StringLiteralExpressionSemanticAction(char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_STRING_LITERAL;
	expression->value = value;
	return expression;
}

Expression * FunctionCallExpressionSemanticAction(FunctionCall * functionCall) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_FUNCTION_CALL;
	expression->functionCall = functionCall;
	return expression;
}

ExpressionList * SingletonExpressionListSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExpressionList * expressionList = calloc(1, sizeof(ExpressionList));
	expressionList->expression = expression;
	return expressionList;
}

ExpressionList * AppendExpressionListSemanticAction(ExpressionList * expressionList, Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ExpressionList * tail = expressionList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonExpressionListSemanticAction(expression);
	return expressionList;
}

TopLevelItem * VariableDeclarationTopLevelItemSemanticAction(VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_VARIABLE_DECLARATION;
	item->variableDeclaration = declaration;
	return item;
}

TopLevelItem * FunctionDeclarationTopLevelItemSemanticAction(FunctionDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_FUNCTION_DECLARATION;
	item->functionDeclaration = declaration;
	return item;
}

TopLevelItem * AggregateDeclarationTopLevelItemSemanticAction(AggregateDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_AGGREGATE_DECLARATION;
	item->aggregateDeclaration = declaration;
	return item;
}

TopLevelItem * EnumDeclarationTopLevelItemSemanticAction(EnumDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_ENUM_DECLARATION;
	item->enumDeclaration = declaration;
	return item;
}

TopLevelItem * TypedefDeclarationTopLevelItemSemanticAction(TypedefDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_TYPEDEF_DECLARATION;
	item->typedefDeclaration = declaration;
	return item;
}

TopLevelItem * FunctionCallTopLevelItemSemanticAction(FunctionCall * functionCall) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_FUNCTION_CALL;
	item->functionCall = functionCall;
	return item;
}

TopLevelItem * EmptyStatementTopLevelItemSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItem * item = calloc(1, sizeof(TopLevelItem));
	item->kind = TOP_LEVEL_EMPTY_STATEMENT;
	return item;
}

TopLevelItemList * SingletonTopLevelItemListSemanticAction(TopLevelItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItemList * itemList = calloc(1, sizeof(TopLevelItemList));
	itemList->item = item;
	return itemList;
}

TopLevelItemList * AppendTopLevelItemListSemanticAction(TopLevelItemList * itemList, TopLevelItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	TopLevelItemList * tail = itemList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonTopLevelItemListSemanticAction(item);
	return itemList;
}

Program * ProgramSemanticAction(TopLevelItemList * items) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->items = items;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}
