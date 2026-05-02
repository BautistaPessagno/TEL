#include "BisonActions.h"

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
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

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS */

Type * TypeSemanticAction(TypeKind kind) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = kind;
	return type;
}

VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclaration * declaration = calloc(1, sizeof(VariableDeclaration));
	declaration->name = name;
	declaration->type = type;
	return declaration;
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
