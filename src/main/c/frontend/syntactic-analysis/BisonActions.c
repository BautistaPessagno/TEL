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
static void _convertExpressionStatementToImplicitReturn(Statement * statement);

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

static void _convertExpressionStatementToImplicitReturn(Statement * statement) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	statement->kind = STATEMENT_RETURN;
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

VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type, Expression * initializer) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclaration * declaration = calloc(1, sizeof(VariableDeclaration));
	declaration->name = name;
	declaration->type = type;
	declaration->initializer = initializer;
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

FunctionDeclaration * FunctionDeclarationSemanticAction(char * name, ParameterList * parameters, Type * returnType, StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	FunctionDeclaration * declaration = calloc(1, sizeof(FunctionDeclaration));
	declaration->name = name;
	declaration->parameters = parameters;
	declaration->returnType = returnType;
	declaration->body = body;
	return declaration;
}

MainDeclaration * MainDeclarationSemanticAction(StatementList * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	MainDeclaration * declaration = calloc(1, sizeof(MainDeclaration));
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

PreprocessorDirective * PreprocessorDirectiveSemanticAction(PreprocessorDirectiveKind kind, char * value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PreprocessorDirective * directive = calloc(1, sizeof(PreprocessorDirective));
	directive->kind = kind;
	directive->value = value;
	return directive;
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

Expression * BinaryExpressionSemanticAction(Expression * left, ExpressionOperator operator, Expression * right) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_BINARY_OPERATION;
	expression->operator = operator;
	expression->left = left;
	expression->right = right;
	return expression;
}

Expression * UnaryExpressionSemanticAction(ExpressionOperator operator, Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->kind = EXPRESSION_UNARY_OPERATION;
	expression->operator = operator;
	expression->operand = operand;
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

ProgramItem * VariableDeclarationProgramItemSemanticAction(VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_VARIABLE_DECLARATION;
	item->variableDeclaration = declaration;
	return item;
}

ProgramItem * FunctionDeclarationProgramItemSemanticAction(FunctionDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_FUNCTION_DECLARATION;
	item->functionDeclaration = declaration;
	return item;
}

ProgramItem * MainDeclarationProgramItemSemanticAction(MainDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_MAIN_DECLARATION;
	item->mainDeclaration = declaration;
	return item;
}

ProgramItem * AggregateDeclarationProgramItemSemanticAction(AggregateDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_AGGREGATE_DECLARATION;
	item->aggregateDeclaration = declaration;
	return item;
}

ProgramItem * EnumDeclarationProgramItemSemanticAction(EnumDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_ENUM_DECLARATION;
	item->enumDeclaration = declaration;
	return item;
}

ProgramItem * TypedefDeclarationProgramItemSemanticAction(TypedefDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_TYPEDEF_DECLARATION;
	item->typedefDeclaration = declaration;
	return item;
}

ProgramItem * PreprocessorDirectiveProgramItemSemanticAction(PreprocessorDirective * directive) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE;
	item->preprocessorDirective = directive;
	return item;
}

ProgramItem * EmptyProgramItemSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItem * item = calloc(1, sizeof(ProgramItem));
	item->kind = PROGRAM_ITEM_EMPTY;
	return item;
}

ProgramItemList * SingletonProgramItemListSemanticAction(ProgramItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItemList * itemList = calloc(1, sizeof(ProgramItemList));
	itemList->item = item;
	return itemList;
}

ProgramItemList * AppendProgramItemListSemanticAction(ProgramItemList * itemList, ProgramItem * item) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ProgramItemList * tail = itemList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonProgramItemListSemanticAction(item);
	return itemList;
}

static int _isImplicitReturnExpression(const Expression * expression) {
	if (expression == NULL) return 0;
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
			return 1;
		case EXPRESSION_BINARY_OPERATION:
			switch (expression->operator) {
				case EXPRESSION_OPERATOR_EQUAL:
				case EXPRESSION_OPERATOR_NOT_EQUAL:
				case EXPRESSION_OPERATOR_LESS_THAN:
				case EXPRESSION_OPERATOR_GREATER_THAN:
				case EXPRESSION_OPERATOR_LESS_EQUAL:
				case EXPRESSION_OPERATOR_GREATER_EQUAL:
				case EXPRESSION_OPERATOR_LOGICAL_OR:
				case EXPRESSION_OPERATOR_LOGICAL_AND:
					return 1;
				default:
					return 0;
			}
		case EXPRESSION_UNARY_OPERATION:
			return expression->operator == EXPRESSION_OPERATOR_LOGICAL_NOT;
		default:
			return 0;
	}
}

Statement * VariableDeclarationStatementSemanticAction(VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_VARIABLE_DECLARATION;
	statement->variableDeclaration = declaration;
	return statement;
}

Statement * ReturnStatementSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_RETURN;
	statement->expression = expression;
	return statement;
}

Statement * ExpressionStatementSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_EXPRESSION;
	statement->expression = expression;
	return statement;
}

Statement * EmptyStatementSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * statement = calloc(1, sizeof(Statement));
	statement->kind = STATEMENT_EMPTY;
	return statement;
}

StatementList * SingletonStatementListSemanticAction(Statement * statement) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	StatementList * statementList = calloc(1, sizeof(StatementList));
	statementList->statement = statement;
	return statementList;
}

StatementList * AppendStatementListSemanticAction(StatementList * statementList, Statement * statement) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	StatementList * tail = statementList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonStatementListSemanticAction(statement);
	return statementList;
}

StatementList * FunctionBodyStatementListSemanticAction(StatementList * statementList) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Statement * lastStatement = NULL;
	StatementList * tail = statementList;
	while (tail != NULL) {
		if (tail->statement != NULL && tail->statement->kind != STATEMENT_EMPTY) {
			lastStatement = tail->statement;
		}
		tail = tail->next;
	}
	if (lastStatement != NULL
		&& lastStatement->kind == STATEMENT_EXPRESSION
		&& _isImplicitReturnExpression(lastStatement->expression)) {
		_convertExpressionStatementToImplicitReturn(lastStatement);
	}
	return statementList;
}

Program * ProgramSemanticAction(ProgramItemList * items) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->items = items;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}
