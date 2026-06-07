#include "SemanticAnalyzer.h"
#include "SemanticSymbolTable.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

typedef struct {
	SemanticSymbolTable * symbols;
	bool hasErrors;
} SemanticAnalysisContext;

/** Shutdown module's internal state. */
void _shutdownSemanticAnalyzerModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeSemanticAnalyzerModule() {
	_logger = createLogger("SemanticAnalyzer");
	return _shutdownSemanticAnalyzerModule;
}

/* PRIVATE FUNCTIONS */

static void _reportSemanticError(SemanticAnalysisContext * context, const char * message, const char * name);
static void _collectGlobalSymbols(SemanticAnalysisContext * context, Program * program);
static void _collectGlobalItem(SemanticAnalysisContext * context, ProgramItem * item);
static void _declareGlobalFunction(SemanticAnalysisContext * context, FunctionDeclaration * declaration);
static void _validateProgram(SemanticAnalysisContext * context, Program * program);
static void _validateProgramItem(SemanticAnalysisContext * context, ProgramItem * item);
static void _validateFunctionDeclaration(SemanticAnalysisContext * context, FunctionDeclaration * declaration);
static void _validateStatementList(SemanticAnalysisContext * context, StatementList * statementList);
static void _validateStatementListInNewScope(SemanticAnalysisContext * context, StatementList * statementList);
static void _validateStatement(SemanticAnalysisContext * context, Statement * statement);
static void _validateVariableDeclaration(SemanticAnalysisContext * context, VariableDeclaration * declaration, bool declareSymbol);
static void _validateAggregateDeclaration(SemanticAnalysisContext * context, AggregateDeclaration * declaration);
static void _validateEnumDeclaration(SemanticAnalysisContext * context, EnumDeclaration * declaration);
static void _validateTypedefDeclaration(SemanticAnalysisContext * context, TypedefDeclaration * declaration);
static void _validateIfStatement(SemanticAnalysisContext * context, IfStatement * statement);
static void _validateForStatement(SemanticAnalysisContext * context, ForStatement * statement);
static void _validateWhileStatement(SemanticAnalysisContext * context, WhileStatement * statement);
static void _validateDoWhileStatement(SemanticAnalysisContext * context, DoWhileStatement * statement);
static void _validateSwitchStatement(SemanticAnalysisContext * context, SwitchStatement * statement);
static void _validateType(SemanticAnalysisContext * context, Type * type);
static void _validateParameterTypes(SemanticAnalysisContext * context, ParameterList * parameters);
static void _validateExpression(SemanticAnalysisContext * context, Expression * expression);
static void _validateExpressionList(SemanticAnalysisContext * context, ExpressionList * expressionList);
static bool _functionSignaturesEqual(SemanticAnalysisContext * context, SemanticSymbol * symbol, FunctionDeclaration * declaration);
static bool _parameterTypesEqual(SemanticAnalysisContext * context, ParameterList * left, ParameterList * right);
static bool _typesEqual(SemanticAnalysisContext * context, Type * left, Type * right);
static Type * _resolveTypedef(SemanticAnalysisContext * context, Type * type);
static bool _effectiveConst(SemanticAnalysisContext * context, Type * type);
static bool _expressionsEqual(Expression * left, Expression * right);
static bool _isCallableSymbol(SemanticAnalysisContext * context, SemanticSymbol * symbol);
static const char * _implicitLoopIteratorName(ForStatement * statement);

static void _reportSemanticError(SemanticAnalysisContext * context, const char * message, const char * name) {
	context->hasErrors = true;
	if (name == NULL) {
		logError(_logger, "%s", message);
	}
	else {
		logError(_logger, "%s: %s", message, name);
	}
}

static void _collectGlobalSymbols(SemanticAnalysisContext * context, Program * program) {
	for (ProgramItemList * item = program->items; item != NULL; item = item->next) {
		_collectGlobalItem(context, item->item);
	}
}

static void _collectGlobalItem(SemanticAnalysisContext * context, ProgramItem * item) {
	if (item == NULL) {
		return;
	}
	switch (item->kind) {
		case PROGRAM_ITEM_VARIABLE_DECLARATION:
			if (!semanticSymbolTableDeclareOrdinary(
					context->symbols,
					item->variableDeclaration->name,
					SEMANTIC_SYMBOL_VARIABLE,
					item->variableDeclaration->type,
					NULL,
					NULL,
					false)) {
				_reportSemanticError(context, "Duplicate global symbol", item->variableDeclaration->name);
			}
			break;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			_declareGlobalFunction(context, item->functionDeclaration);
			break;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION: {
			SemanticTagKind kind = item->aggregateDeclaration->kind == AGGREGATE_STRUCT_KIND
				? SEMANTIC_TAG_STRUCT
				: SEMANTIC_TAG_UNION;
			if (!semanticSymbolTableDeclareTag(context->symbols, item->aggregateDeclaration->name, kind)) {
				_reportSemanticError(context, "Duplicate tag", item->aggregateDeclaration->name);
			}
			break;
		}
		case PROGRAM_ITEM_ENUM_DECLARATION:
			if (!semanticSymbolTableDeclareTag(context->symbols, item->enumDeclaration->name, SEMANTIC_TAG_ENUM)) {
				_reportSemanticError(context, "Duplicate tag", item->enumDeclaration->name);
			}
			for (EnumMemberList * member = item->enumDeclaration->members; member != NULL; member = member->next) {
				if (!semanticSymbolTableDeclareOrdinary(
						context->symbols,
						member->member->name,
						SEMANTIC_SYMBOL_ENUM_CONSTANT,
						NULL,
						NULL,
						NULL,
						false)) {
					_reportSemanticError(context, "Duplicate enum constant", member->member->name);
				}
			}
			break;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION:
			if (!semanticSymbolTableDeclareOrdinary(
					context->symbols,
					item->typedefDeclaration->name,
					SEMANTIC_SYMBOL_TYPEDEF,
					item->typedefDeclaration->type,
					NULL,
					NULL,
					false)) {
				_reportSemanticError(context, "Duplicate global symbol", item->typedefDeclaration->name);
			}
			break;
		default:
			break;
	}
}

static void _declareGlobalFunction(SemanticAnalysisContext * context, FunctionDeclaration * declaration) {
	SemanticSymbol * existing = semanticSymbolTableLookupCurrentOrdinary(context->symbols, declaration->name);
	bool hasDefinition = declaration->body != NULL;
	if (existing == NULL) {
		semanticSymbolTableDeclareOrdinary(
			context->symbols,
			declaration->name,
			SEMANTIC_SYMBOL_FUNCTION,
			NULL,
			declaration->parameters,
			declaration->returnType,
			hasDefinition);
		return;
	}
	if (existing->kind != SEMANTIC_SYMBOL_FUNCTION) {
		_reportSemanticError(context, "Duplicate global symbol", declaration->name);
		return;
	}
	if (!_functionSignaturesEqual(context, existing, declaration)) {
		_reportSemanticError(context, "Incompatible function redeclaration", declaration->name);
		return;
	}
	if (existing->hasDefinition && hasDefinition) {
		_reportSemanticError(context, "Duplicate function definition", declaration->name);
		return;
	}
	if (hasDefinition) {
		existing->hasDefinition = true;
	}
}

static void _validateProgram(SemanticAnalysisContext * context, Program * program) {
	for (ProgramItemList * item = program->items; item != NULL; item = item->next) {
		_validateProgramItem(context, item->item);
	}
}

static void _validateProgramItem(SemanticAnalysisContext * context, ProgramItem * item) {
	if (item == NULL) {
		return;
	}
	switch (item->kind) {
		case PROGRAM_ITEM_VARIABLE_DECLARATION:
			_validateVariableDeclaration(context, item->variableDeclaration, false);
			break;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			_validateFunctionDeclaration(context, item->functionDeclaration);
			break;
		case PROGRAM_ITEM_MAIN_DECLARATION:
			_validateStatementListInNewScope(context, item->mainDeclaration->body);
			break;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION:
			_validateAggregateDeclaration(context, item->aggregateDeclaration);
			break;
		case PROGRAM_ITEM_ENUM_DECLARATION:
			_validateEnumDeclaration(context, item->enumDeclaration);
			break;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION:
			_validateTypedefDeclaration(context, item->typedefDeclaration);
			break;
		default:
			break;
	}
}

static void _validateFunctionDeclaration(SemanticAnalysisContext * context, FunctionDeclaration * declaration) {
	_validateType(context, declaration->returnType);
	semanticSymbolTablePushScope(context->symbols);
	for (ParameterList * parameter = declaration->parameters; parameter != NULL; parameter = parameter->next) {
		_validateType(context, parameter->parameter->type);
		if (parameter->parameter->name != NULL
			&& !semanticSymbolTableDeclareOrdinary(
				context->symbols,
				parameter->parameter->name,
				SEMANTIC_SYMBOL_VARIABLE,
				parameter->parameter->type,
				NULL,
				NULL,
				false)) {
			_reportSemanticError(context, "Duplicate parameter", parameter->parameter->name);
		}
	}
	if (declaration->body != NULL) {
		_validateStatementList(context, declaration->body);
	}
	semanticSymbolTablePopScope(context->symbols);
}

static void _validateStatementList(SemanticAnalysisContext * context, StatementList * statementList) {
	for (StatementList * item = statementList; item != NULL; item = item->next) {
		_validateStatement(context, item->statement);
	}
}

static void _validateStatementListInNewScope(SemanticAnalysisContext * context, StatementList * statementList) {
	semanticSymbolTablePushScope(context->symbols);
	_validateStatementList(context, statementList);
	semanticSymbolTablePopScope(context->symbols);
}

static void _validateStatement(SemanticAnalysisContext * context, Statement * statement) {
	if (statement == NULL) {
		return;
	}
	switch (statement->kind) {
		case STATEMENT_VARIABLE_DECLARATION:
			_validateVariableDeclaration(context, statement->variableDeclaration, true);
			break;
		case STATEMENT_RETURN:
		case STATEMENT_EXPRESSION:
			_validateExpression(context, statement->expression);
			break;
		case STATEMENT_IF:
			_validateIfStatement(context, statement->ifStatement);
			break;
		case STATEMENT_FOR:
			_validateForStatement(context, statement->forStatement);
			break;
		case STATEMENT_WHILE:
			_validateWhileStatement(context, statement->whileStatement);
			break;
		case STATEMENT_DO_WHILE:
			_validateDoWhileStatement(context, statement->doWhileStatement);
			break;
		case STATEMENT_SWITCH:
			_validateSwitchStatement(context, statement->switchStatement);
			break;
		default:
			break;
	}
}

static void _validateVariableDeclaration(SemanticAnalysisContext * context, VariableDeclaration * declaration, bool declareSymbol) {
	_validateType(context, declaration->type);
	_validateExpression(context, declaration->initializer);
	if (declareSymbol
		&& !semanticSymbolTableDeclareOrdinary(
			context->symbols,
			declaration->name,
			SEMANTIC_SYMBOL_VARIABLE,
			declaration->type,
			NULL,
			NULL,
			false)) {
		_reportSemanticError(context, "Duplicate local symbol", declaration->name);
	}
}

static void _validateAggregateDeclaration(SemanticAnalysisContext * context, AggregateDeclaration * declaration) {
	for (VariableDeclarationList * field = declaration->fields; field != NULL; field = field->next) {
		_validateVariableDeclaration(context, field->declaration, false);
	}
}

static void _validateEnumDeclaration(SemanticAnalysisContext * context, EnumDeclaration * declaration) {
	(void) context;
	(void) declaration;
}

static void _validateTypedefDeclaration(SemanticAnalysisContext * context, TypedefDeclaration * declaration) {
	_validateType(context, declaration->type);
}

static void _validateIfStatement(SemanticAnalysisContext * context, IfStatement * statement) {
	for (IfBranch * branch = statement->branches; branch != NULL; branch = branch->next) {
		_validateExpression(context, branch->condition);
		_validateStatementListInNewScope(context, branch->body);
	}
	if (statement->elseBody != NULL) {
		_validateStatementListInNewScope(context, statement->elseBody);
	}
}

static void _validateForStatement(SemanticAnalysisContext * context, ForStatement * statement) {
	semanticSymbolTablePushScope(context->symbols);
	const char * iteratorName = _implicitLoopIteratorName(statement);
	if (iteratorName != NULL && semanticSymbolTableLookupOrdinary(context->symbols, iteratorName) == NULL) {
		semanticSymbolTableDeclareOrdinary(
			context->symbols,
			iteratorName,
			SEMANTIC_SYMBOL_VARIABLE,
			NULL,
			NULL,
			NULL,
			false);
	}
	_validateExpression(context, statement->initializer);
	_validateExpression(context, statement->condition);
	_validateExpression(context, statement->update);
	_validateStatementListInNewScope(context, statement->body);
	semanticSymbolTablePopScope(context->symbols);
}

static void _validateWhileStatement(SemanticAnalysisContext * context, WhileStatement * statement) {
	_validateExpression(context, statement->condition);
	_validateStatementListInNewScope(context, statement->body);
}

static void _validateDoWhileStatement(SemanticAnalysisContext * context, DoWhileStatement * statement) {
	_validateStatementListInNewScope(context, statement->body);
	_validateExpression(context, statement->condition);
}

static void _validateSwitchStatement(SemanticAnalysisContext * context, SwitchStatement * statement) {
	_validateExpression(context, statement->discriminant);
	for (SwitchCase * switchCase = statement->cases; switchCase != NULL; switchCase = switchCase->next) {
		_validateExpression(context, switchCase->matchExpression);
		_validateStatementListInNewScope(context, switchCase->body);
	}
}

static void _validateType(SemanticAnalysisContext * context, Type * type) {
	if (type == NULL) {
		return;
	}
	switch (type->kind) {
		case TYPE_NAMED_KIND: {
			SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, type->name);
			if (symbol == NULL || symbol->kind != SEMANTIC_SYMBOL_TYPEDEF) {
				_reportSemanticError(context, "Unknown typedef", type->name);
			}
			break;
		}
		case TYPE_STRUCT_KIND: {
			SemanticTag * tag = semanticSymbolTableLookupTag(context->symbols, type->name);
			if (tag == NULL || tag->kind != SEMANTIC_TAG_STRUCT) {
				_reportSemanticError(context, "Unknown struct type", type->name);
			}
			break;
		}
		case TYPE_ENUM_KIND: {
			SemanticTag * tag = semanticSymbolTableLookupTag(context->symbols, type->name);
			if (tag == NULL || tag->kind != SEMANTIC_TAG_ENUM) {
				_reportSemanticError(context, "Unknown enum type", type->name);
			}
			break;
		}
		case TYPE_UNION_KIND: {
			SemanticTag * tag = semanticSymbolTableLookupTag(context->symbols, type->name);
			if (tag == NULL || tag->kind != SEMANTIC_TAG_UNION) {
				_reportSemanticError(context, "Unknown union type", type->name);
			}
			break;
		}
		case TYPE_POINTER_KIND:
		case TYPE_ARRAY_KIND:
			_validateType(context, type->pointee);
			_validateExpression(context, type->arraySize);
			break;
		case TYPE_FUNCTION_POINTER_KIND:
			_validateParameterTypes(context, type->functionParams);
			_validateType(context, type->returnType);
			break;
		default:
			break;
	}
}

static void _validateParameterTypes(SemanticAnalysisContext * context, ParameterList * parameters) {
	for (ParameterList * parameter = parameters; parameter != NULL; parameter = parameter->next) {
		_validateType(context, parameter->parameter->type);
	}
}

static void _validateExpression(SemanticAnalysisContext * context, Expression * expression) {
	if (expression == NULL) {
		return;
	}
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
			if (semanticSymbolTableLookupOrdinary(context->symbols, expression->value) == NULL) {
				_reportSemanticError(context, "Unknown identifier", expression->value);
			}
			break;
		case EXPRESSION_FUNCTION_CALL: {
			SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, expression->functionCall->name);
			if (!_isCallableSymbol(context, symbol)) {
				_reportSemanticError(context, "Unknown function", expression->functionCall->name);
			}
			_validateExpressionList(context, expression->functionCall->arguments);
			break;
		}
		case EXPRESSION_BINARY_OPERATION:
			switch (expression->operator) {
				case EXPRESSION_OPERATOR_MEMBER_ACCESS:
				case EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS:
					_validateExpression(context, expression->left);
					break;
				default:
					_validateExpression(context, expression->left);
					_validateExpression(context, expression->right);
					break;
			}
			break;
		case EXPRESSION_UNARY_OPERATION:
			_validateExpression(context, expression->operand);
			break;
		case EXPRESSION_ARRAY_LITERAL:
			_validateExpressionList(context, expression->elements);
			break;
		default:
			break;
	}
}

static void _validateExpressionList(SemanticAnalysisContext * context, ExpressionList * expressionList) {
	for (ExpressionList * item = expressionList; item != NULL; item = item->next) {
		_validateExpression(context, item->expression);
	}
}

static bool _functionSignaturesEqual(SemanticAnalysisContext * context, SemanticSymbol * symbol, FunctionDeclaration * declaration) {
	return _typesEqual(context, symbol->returnType, declaration->returnType)
		&& _parameterTypesEqual(context, symbol->parameters, declaration->parameters);
}

static bool _parameterTypesEqual(SemanticAnalysisContext * context, ParameterList * left, ParameterList * right) {
	while (left != NULL && right != NULL) {
		if (!_typesEqual(context, left->parameter->type, right->parameter->type)) {
			return false;
		}
		left = left->next;
		right = right->next;
	}
	return left == NULL && right == NULL;
}

static bool _typesEqual(SemanticAnalysisContext * context, Type * left, Type * right) {
	if (left == NULL || right == NULL) {
		return left == right;
	}
	Type * resolvedLeft = _resolveTypedef(context, left);
	Type * resolvedRight = _resolveTypedef(context, right);
	if (resolvedLeft == NULL || resolvedRight == NULL) {
		return resolvedLeft == resolvedRight;
	}
	if (resolvedLeft->kind != resolvedRight->kind
		|| _effectiveConst(context, left) != _effectiveConst(context, right)) {
		return false;
	}
	switch (resolvedLeft->kind) {
		case TYPE_NAMED_KIND:
		case TYPE_STRUCT_KIND:
		case TYPE_ENUM_KIND:
		case TYPE_UNION_KIND:
			if (resolvedLeft->name == NULL || resolvedRight->name == NULL) {
				return resolvedLeft->name == resolvedRight->name;
			}
			return strcmp(resolvedLeft->name, resolvedRight->name) == 0;
		case TYPE_POINTER_KIND:
		case TYPE_ARRAY_KIND:
			return _typesEqual(context, resolvedLeft->pointee, resolvedRight->pointee)
				&& _expressionsEqual(resolvedLeft->arraySize, resolvedRight->arraySize);
		case TYPE_FUNCTION_POINTER_KIND:
			return _parameterTypesEqual(context, resolvedLeft->functionParams, resolvedRight->functionParams)
				&& _typesEqual(context, resolvedLeft->returnType, resolvedRight->returnType);
		default:
			return true;
	}
}

static Type * _resolveTypedef(SemanticAnalysisContext * context, Type * type) {
	Type * resolved = type;
	while (resolved != NULL && resolved->kind == TYPE_NAMED_KIND) {
		SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, resolved->name);
		if (symbol == NULL || symbol->kind != SEMANTIC_SYMBOL_TYPEDEF || symbol->type == NULL) {
			return resolved;
		}
		resolved = symbol->type;
	}
	return resolved;
}

static bool _effectiveConst(SemanticAnalysisContext * context, Type * type) {
	bool isConst = false;
	while (type != NULL) {
		isConst = isConst || type->isConst;
		if (type->kind != TYPE_NAMED_KIND) {
			return isConst;
		}
		SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, type->name);
		if (symbol == NULL || symbol->kind != SEMANTIC_SYMBOL_TYPEDEF || symbol->type == NULL) {
			return isConst;
		}
		type = symbol->type;
	}
	return isConst;
}

static bool _expressionsEqual(Expression * left, Expression * right) {
	if (left == NULL || right == NULL) {
		return left == right;
	}
	if (left->kind != right->kind) {
		return false;
	}
	switch (left->kind) {
		case EXPRESSION_IDENTIFIER:
		case EXPRESSION_INTEGER_LITERAL:
		case EXPRESSION_FLOAT_LITERAL:
		case EXPRESSION_CHAR_LITERAL:
		case EXPRESSION_STRING_LITERAL:
			if (left->value == NULL || right->value == NULL) {
				return left->value == right->value;
			}
			return strcmp(left->value, right->value) == 0;
		case EXPRESSION_BINARY_OPERATION:
			return left->operator == right->operator
				&& _expressionsEqual(left->left, right->left)
				&& _expressionsEqual(left->right, right->right);
		case EXPRESSION_UNARY_OPERATION:
			return left->operator == right->operator
				&& _expressionsEqual(left->operand, right->operand);
		case EXPRESSION_NULL_LITERAL:
			return true;
		default:
			return false;
	}
}

static bool _isCallableSymbol(SemanticAnalysisContext * context, SemanticSymbol * symbol) {
	if (symbol == NULL) {
		return false;
	}
	if (symbol->kind == SEMANTIC_SYMBOL_FUNCTION) {
		return true;
	}
	if (symbol->kind != SEMANTIC_SYMBOL_VARIABLE) {
		return false;
	}
	Type * type = _resolveTypedef(context, symbol->type);
	return type != NULL && type->kind == TYPE_FUNCTION_POINTER_KIND;
}

static const char * _implicitLoopIteratorName(ForStatement * statement) {
	if (statement == NULL
		|| statement->initializer == NULL
		|| statement->initializer->kind != EXPRESSION_BINARY_OPERATION
		|| statement->initializer->operator != EXPRESSION_OPERATOR_ASSIGN
		|| statement->initializer->left == NULL
		|| statement->initializer->left->kind != EXPRESSION_IDENTIFIER) {
		return NULL;
	}
	return statement->initializer->left->value;
}

/** PUBLIC FUNCTIONS */

CompilationStatus executeSemanticAnalysis(CompilerState * compilerState) {
	logDebugging(_logger, "Executing semantic analysis...");
	if (compilerState == NULL || compilerState->abstractSyntaxtTree == NULL) {
		logError(_logger, "Missing AST for semantic analysis.");
		return FAILED;
	}

	SemanticAnalysisContext context = {
		.symbols = createSemanticSymbolTable(),
		.hasErrors = false
	};
	semanticSymbolTablePushScope(context.symbols);

	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	_collectGlobalSymbols(&context, program);
	_validateProgram(&context, program);

	destroySemanticSymbolTable(context.symbols);
	return context.hasErrors ? FAILED : SUCCEEDED;
}
