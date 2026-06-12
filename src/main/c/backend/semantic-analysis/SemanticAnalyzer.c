#include "SemanticAnalyzer.h"
#include "SemanticSymbolTable.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

typedef struct SemanticTemporaryType SemanticTemporaryType;

struct SemanticTemporaryType {
	Type type;
	SemanticTemporaryType * next;
};

typedef struct {
	SemanticSymbolTable * symbols;
	SemanticTemporaryType * temporaryTypes;
	Type * currentReturnType;
	size_t loopDepth;
	size_t switchDepth;
	bool hasErrors;
} SemanticAnalysisContext;

typedef struct {
	Type * type;
	SemanticSymbol * symbol;
	bool isLvalue;
	bool isReadonlyLvalue;
	bool isFunctionDesignator;
} SemanticExpressionInfo;

typedef enum {
	SEMANTIC_TYPE_VARIABLE,
	SEMANTIC_TYPE_PARAMETER,
	SEMANTIC_TYPE_FIELD,
	SEMANTIC_TYPE_RETURN,
	SEMANTIC_TYPE_TYPEDEF
} SemanticTypeContext;

/* Shared sentinel types; callers must treat returned Type * values as immutable. */
static Type _semanticIntType = { .kind = TYPE_INT_KIND };
static Type _semanticFloatType = { .kind = TYPE_FLOAT_KIND };
static Type _semanticCharType = { .kind = TYPE_CHAR_KIND };
static Type _semanticVoidType = { .kind = TYPE_VOID_KIND };

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
static bool _validateStatementList(SemanticAnalysisContext * context, StatementList * statementList);
static bool _validateStatementListInNewScope(SemanticAnalysisContext * context, StatementList * statementList);
static bool _validateStatement(SemanticAnalysisContext * context, Statement * statement);
static bool _validateReturnStatement(SemanticAnalysisContext * context, Statement * statement);
static void _validateVariableDeclaration(SemanticAnalysisContext * context, VariableDeclaration * declaration, bool declareSymbol);
static void _validateAggregateDeclaration(SemanticAnalysisContext * context, AggregateDeclaration * declaration);
static void _validateEnumDeclaration(SemanticAnalysisContext * context, EnumDeclaration * declaration);
static void _validateTypedefDeclaration(SemanticAnalysisContext * context, TypedefDeclaration * declaration);
static bool _validateIfStatement(SemanticAnalysisContext * context, IfStatement * statement);
static void _validateForStatement(SemanticAnalysisContext * context, ForStatement * statement);
static void _validateWhileStatement(SemanticAnalysisContext * context, WhileStatement * statement);
static void _validateDoWhileStatement(SemanticAnalysisContext * context, DoWhileStatement * statement);
static void _validateSwitchStatement(SemanticAnalysisContext * context, SwitchStatement * statement);
static void _validateType(SemanticAnalysisContext * context, Type * type, SemanticTypeContext typeContext);
static void _validateTypeNode(
	SemanticAnalysisContext * context,
	Type * type,
	SemanticTypeContext typeContext,
	bool nestedUnderIndirection);
static void _validateParameterTypes(SemanticAnalysisContext * context, ParameterList * parameters);
static void _validateExpression(SemanticAnalysisContext * context, Expression * expression);
static void _validateExpressionList(SemanticAnalysisContext * context, ExpressionList * expressionList);
static void _validateConditionExpression(SemanticAnalysisContext * context, Expression * expression, const char * message);
static void _validateSwitchExpression(SemanticAnalysisContext * context, Expression * expression, const char * message);
static Type * _createTemporaryType(SemanticAnalysisContext * context, TypeKind kind);
static void _destroyTemporaryTypes(SemanticTemporaryType * type);
static SemanticExpressionInfo _expressionInfo(SemanticAnalysisContext * context, Expression * expression);
static SemanticExpressionInfo _expressionInfoForIdentifier(SemanticAnalysisContext * context, Expression * expression);
static SemanticExpressionInfo _expressionInfoForFunctionCall(SemanticAnalysisContext * context, FunctionCall * functionCall);
static SemanticExpressionInfo _expressionInfoForBinaryOperation(SemanticAnalysisContext * context, Expression * expression);
static SemanticExpressionInfo _expressionInfoForUnaryOperation(SemanticAnalysisContext * context, Expression * expression);
static SemanticExpressionInfo _expressionInfoWithType(Type * type, bool isLvalue);
static Type * _assignmentTargetType(SemanticAnalysisContext * context, Expression * expression);
static Type * _arrayIndexElementType(SemanticAnalysisContext * context, Type * indexedType);
static Type * _memberAccessType(SemanticAnalysisContext * context, Type * receiverType, const char * fieldName);
static VariableDeclaration * _findAggregateField(AggregateDeclaration * declaration, const char * fieldName);
static bool _validateInitializer(SemanticAnalysisContext * context, VariableDeclaration * declaration);
static bool _isExpressionAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression);
static bool _isArrayLiteralAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression);
static bool _isAddressExpressionAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression);
static bool _isFunctionDesignatorAssignableToType(SemanticAnalysisContext * context, Type * targetType, SemanticSymbol * symbol);
static bool _isStringLiteralAssignableToType(SemanticAnalysisContext * context, Type * targetType);
static bool _isNumericLiteral(Expression * expression);
static bool _isNumericScalarType(SemanticAnalysisContext * context, Type * type);
static bool _isIntegerScalarType(SemanticAnalysisContext * context, Type * type);
static bool _isPointerLikeType(SemanticAnalysisContext * context, Type * type);
static bool _isConditionType(SemanticAnalysisContext * context, Type * type);
static bool _isVoidType(SemanticAnalysisContext * context, Type * type);
static bool _isDirectVoidType(SemanticAnalysisContext * context, Type * type);
static bool _isNullLiteral(Expression * expression);
static bool _isEqualityComparable(SemanticAnalysisContext * context, Expression * left, Type * leftType, Expression * right, Type * rightType);
static bool _typesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType);
static bool _pointerTypesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType);
static bool _pointeeTypesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType);
static bool _typesEqualIgnoringTopConst(SemanticAnalysisContext * context, Type * left, Type * right);
static void _validateCallArguments(SemanticAnalysisContext * context, FunctionCall * functionCall, SemanticSymbol * symbol);
static size_t _expressionListLength(ExpressionList * expressionList);
static bool _fixedArrayBound(Expression * expression, size_t * bound);
static bool _parseArrayBoundLiteral(const char * literal, size_t * bound);
static bool _isNonPositiveArrayBound(Expression * expression);
static bool _isAssignmentOperator(ExpressionOperator operator);
static bool _isCompoundAssignmentOperator(ExpressionOperator operator);
static bool _isNumericBinaryOperator(ExpressionOperator operator);
static bool _isEqualityOperator(ExpressionOperator operator);
static bool _isComparisonOperator(ExpressionOperator operator);
static bool _isLogicalOperator(ExpressionOperator operator);
static bool _functionSignaturesEqual(SemanticAnalysisContext * context, SemanticSymbol * symbol, FunctionDeclaration * declaration);
static bool _callableSignatureMatches(SemanticAnalysisContext * context, SemanticSymbol * symbol, Type * returnType, ParameterList * parameters);
static Type * _callableReturnType(SemanticAnalysisContext * context, SemanticSymbol * symbol);
static ParameterList * _callableParameters(SemanticAnalysisContext * context, SemanticSymbol * symbol);
static bool _parameterTypesEqual(SemanticAnalysisContext * context, ParameterList * left, ParameterList * right);
static bool _typesEqual(SemanticAnalysisContext * context, Type * left, Type * right);
static Type * _resolveTypedef(SemanticAnalysisContext * context, Type * type);
static bool _effectiveConst(SemanticAnalysisContext * context, Type * type);
static bool _expressionsEqual(Expression * left, Expression * right);
static bool _isCallableSymbol(SemanticAnalysisContext * context, SemanticSymbol * symbol);
static const char * _declaredLoopIteratorName(ForStatement * statement);

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
			if (!semanticSymbolTableDeclareTag(
					context->symbols,
					item->aggregateDeclaration->name,
					kind,
					item->aggregateDeclaration)) {
				_reportSemanticError(context, "Duplicate tag", item->aggregateDeclaration->name);
			}
			break;
		}
		case PROGRAM_ITEM_ENUM_DECLARATION:
			if (!semanticSymbolTableDeclareTag(
					context->symbols,
					item->enumDeclaration->name,
					SEMANTIC_TAG_ENUM,
					NULL)) {
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
		case PROGRAM_ITEM_MAIN_DECLARATION: {
			Type * previousReturnType = context->currentReturnType;
			context->currentReturnType = &_semanticIntType;
			(void) _validateStatementListInNewScope(context, item->mainDeclaration->body);
			context->currentReturnType = previousReturnType;
			break;
		}
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
	_validateType(context, declaration->returnType, SEMANTIC_TYPE_RETURN);
	semanticSymbolTablePushScope(context->symbols);
	Type * previousReturnType = context->currentReturnType;
	context->currentReturnType = declaration->returnType;
	for (ParameterList * parameter = declaration->parameters; parameter != NULL; parameter = parameter->next) {
		_validateType(context, parameter->parameter->type, SEMANTIC_TYPE_PARAMETER);
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
		bool guaranteesReturn = _validateStatementList(context, declaration->body);
		if (!_isVoidType(context, declaration->returnType) && !guaranteesReturn) {
			_reportSemanticError(context, "Non-void function does not guarantee a return", declaration->name);
		}
	}
	context->currentReturnType = previousReturnType;
	semanticSymbolTablePopScope(context->symbols);
}

static bool _validateStatementList(SemanticAnalysisContext * context, StatementList * statementList) {
	bool guaranteesReturn = false;
	for (StatementList * item = statementList; item != NULL; item = item->next) {
		guaranteesReturn = _validateStatement(context, item->statement) || guaranteesReturn;
	}
	return guaranteesReturn;
}

static bool _validateStatementListInNewScope(SemanticAnalysisContext * context, StatementList * statementList) {
	semanticSymbolTablePushScope(context->symbols);
	bool guaranteesReturn = _validateStatementList(context, statementList);
	semanticSymbolTablePopScope(context->symbols);
	return guaranteesReturn;
}

static bool _validateStatement(SemanticAnalysisContext * context, Statement * statement) {
	if (statement == NULL) {
		return false;
	}
	switch (statement->kind) {
		case STATEMENT_VARIABLE_DECLARATION:
			_validateVariableDeclaration(context, statement->variableDeclaration, true);
			break;
		case STATEMENT_RETURN:
			return _validateReturnStatement(context, statement);
		case STATEMENT_EXPRESSION:
			_validateExpression(context, statement->expression);
			break;
		case STATEMENT_IF:
			return _validateIfStatement(context, statement->ifStatement);
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
		case STATEMENT_BREAK:
			if (context->loopDepth == 0 && context->switchDepth == 0) {
				_reportSemanticError(context, "Break outside loop or switch", NULL);
			}
			break;
		case STATEMENT_CONTINUE:
			if (context->loopDepth == 0) {
				_reportSemanticError(context, "Continue outside loop", NULL);
			}
			break;
		default:
			break;
	}
	return false;
}

static bool _validateReturnStatement(SemanticAnalysisContext * context, Statement * statement) {
	Type * returnType = context->currentReturnType != NULL ? context->currentReturnType : &_semanticVoidType;
	if (_isVoidType(context, returnType)) {
		if (statement->expression != NULL) {
			_validateExpression(context, statement->expression);
			_reportSemanticError(context, "Void function cannot return a value", NULL);
		}
		return true;
	}
	if (statement->expression == NULL) {
		_reportSemanticError(context, "Non-void function must return a value", NULL);
		return true;
	}
	if (!_isExpressionAssignableToType(context, returnType, statement->expression)) {
		_reportSemanticError(context, "Incompatible return value", NULL);
	}
	return true;
}

static void _validateVariableDeclaration(SemanticAnalysisContext * context, VariableDeclaration * declaration, bool declareSymbol) {
	_validateType(context, declaration->type, SEMANTIC_TYPE_VARIABLE);
	_validateInitializer(context, declaration);
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
		VariableDeclaration * current = field->declaration;
		_validateType(context, current->type, SEMANTIC_TYPE_FIELD);
		if (current->initializer != NULL) {
			_validateExpression(context, current->initializer);
			_reportSemanticError(context, "Aggregate fields cannot have initializers", current->name);
		}
		for (VariableDeclarationList * previous = declaration->fields;
			previous != field;
			previous = previous->next) {
			if (previous->declaration != NULL
				&& strcmp(previous->declaration->name, current->name) == 0) {
				_reportSemanticError(context, "Duplicate aggregate field", current->name);
				break;
			}
		}
	}
}

static void _validateEnumDeclaration(SemanticAnalysisContext * context, EnumDeclaration * declaration) {
	/* Enum values are integer literals by grammar; duplicate numeric values are legal. */
	(void) context;
	(void) declaration;
}

static void _validateTypedefDeclaration(SemanticAnalysisContext * context, TypedefDeclaration * declaration) {
	_validateType(context, declaration->type, SEMANTIC_TYPE_TYPEDEF);
}

static bool _validateIfStatement(SemanticAnalysisContext * context, IfStatement * statement) {
	if (statement == NULL) {
		return false;
	}
	bool allBranchesReturn = statement->branches != NULL && statement->elseBody != NULL;
	for (IfBranch * branch = statement->branches; branch != NULL; branch = branch->next) {
		_validateConditionExpression(context, branch->condition, "If condition must be numeric or pointer-like");
		allBranchesReturn = _validateStatementListInNewScope(context, branch->body) && allBranchesReturn;
	}
	if (statement->elseBody != NULL) {
		allBranchesReturn = _validateStatementListInNewScope(context, statement->elseBody) && allBranchesReturn;
	}
	return allBranchesReturn;
}

static void _validateForStatement(SemanticAnalysisContext * context, ForStatement * statement) {
	semanticSymbolTablePushScope(context->symbols);
	const char * iteratorName = _declaredLoopIteratorName(statement);
	if (iteratorName != NULL) {
		semanticSymbolTableDeclareOrdinary(
			context->symbols,
			iteratorName,
			SEMANTIC_SYMBOL_VARIABLE,
			&_semanticIntType,
			NULL,
			NULL,
			false);
	}
	_validateExpression(context, statement->initializer);
	_validateConditionExpression(context, statement->condition, "For condition must be numeric or pointer-like");
	_validateExpression(context, statement->update);
	context->loopDepth++;
	_validateStatementListInNewScope(context, statement->body);
	context->loopDepth--;
	semanticSymbolTablePopScope(context->symbols);
}

static void _validateWhileStatement(SemanticAnalysisContext * context, WhileStatement * statement) {
	_validateConditionExpression(context, statement->condition, "While condition must be numeric or pointer-like");
	context->loopDepth++;
	_validateStatementListInNewScope(context, statement->body);
	context->loopDepth--;
}

static void _validateDoWhileStatement(SemanticAnalysisContext * context, DoWhileStatement * statement) {
	context->loopDepth++;
	_validateStatementListInNewScope(context, statement->body);
	context->loopDepth--;
	_validateConditionExpression(context, statement->condition, "Do-while condition must be numeric or pointer-like");
}

static void _validateSwitchStatement(SemanticAnalysisContext * context, SwitchStatement * statement) {
	_validateSwitchExpression(context, statement->discriminant, "Switch discriminant must be numeric");
	context->switchDepth++;
	for (SwitchCase * switchCase = statement->cases; switchCase != NULL; switchCase = switchCase->next) {
		_validateSwitchExpression(context, switchCase->matchExpression, "Switch case expression must be numeric");
		_validateStatementListInNewScope(context, switchCase->body);
	}
	context->switchDepth--;
}

static void _validateType(SemanticAnalysisContext * context, Type * type, SemanticTypeContext typeContext) {
	_validateTypeNode(context, type, typeContext, false);
}

static void _validateTypeNode(
	SemanticAnalysisContext * context,
	Type * type,
	SemanticTypeContext typeContext,
	bool nestedUnderIndirection) {
	if (type == NULL) {
		return;
	}
	Type * resolvedType = _resolveTypedef(context, type);
	if (!nestedUnderIndirection
		&& (typeContext == SEMANTIC_TYPE_VARIABLE
			|| typeContext == SEMANTIC_TYPE_PARAMETER
			|| typeContext == SEMANTIC_TYPE_FIELD)
		&& _isDirectVoidType(context, type)) {
		_reportSemanticError(context, "Variables, parameters, and fields cannot have void type", NULL);
	}
	if (!nestedUnderIndirection
		&& typeContext == SEMANTIC_TYPE_RETURN
		&& resolvedType != NULL
		&& resolvedType->kind == TYPE_ARRAY_KIND) {
		_reportSemanticError(context, "Functions cannot return arrays", NULL);
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
			_validateTypeNode(context, type->pointee, typeContext, true);
			break;
		case TYPE_ARRAY_KIND:
			_validateTypeNode(context, type->pointee, typeContext, true);
			if (_isDirectVoidType(context, type->pointee)) {
				_reportSemanticError(context, "Arrays cannot contain void elements", NULL);
			}
			if (type->arraySize == NULL) {
				if (typeContext != SEMANTIC_TYPE_PARAMETER) {
					_reportSemanticError(context, "Unsized arrays are only allowed as parameters", NULL);
				}
			}
			else {
				SemanticExpressionInfo sizeInfo = _expressionInfo(context, type->arraySize);
				if (!_isIntegerScalarType(context, sizeInfo.type)) {
					_reportSemanticError(context, "Array bound must have integer type", NULL);
				}
				if (_isNonPositiveArrayBound(type->arraySize)) {
					_reportSemanticError(context, "Array bound must be greater than zero", NULL);
				}
			}
			break;
		case TYPE_FUNCTION_POINTER_KIND:
			_validateParameterTypes(context, type->functionParams);
			_validateType(context, type->returnType, SEMANTIC_TYPE_RETURN);
			break;
		default:
			break;
	}
}

static void _validateParameterTypes(SemanticAnalysisContext * context, ParameterList * parameters) {
	for (ParameterList * parameter = parameters; parameter != NULL; parameter = parameter->next) {
		_validateType(context, parameter->parameter->type, SEMANTIC_TYPE_PARAMETER);
	}
}

static void _validateExpression(SemanticAnalysisContext * context, Expression * expression) {
	(void) _expressionInfo(context, expression);
}

static void _validateExpressionList(SemanticAnalysisContext * context, ExpressionList * expressionList) {
	for (ExpressionList * item = expressionList; item != NULL; item = item->next) {
		_validateExpression(context, item->expression);
	}
}

static void _validateConditionExpression(SemanticAnalysisContext * context, Expression * expression, const char * message) {
	SemanticExpressionInfo info = _expressionInfo(context, expression);
	if (!_isNullLiteral(expression) && !_isConditionType(context, info.type)) {
		_reportSemanticError(context, message, NULL);
	}
}

static void _validateSwitchExpression(SemanticAnalysisContext * context, Expression * expression, const char * message) {
	if (expression == NULL) {
		return;
	}
	SemanticExpressionInfo info = _expressionInfo(context, expression);
	if (!_isNumericScalarType(context, info.type)) {
		_reportSemanticError(context, message, NULL);
	}
}

static Type * _createTemporaryType(SemanticAnalysisContext * context, TypeKind kind) {
	if (context == NULL) {
		return NULL;
	}
	SemanticTemporaryType * temporary = calloc(1, sizeof(SemanticTemporaryType));
	if (temporary == NULL) {
		_reportSemanticError(context, "Out of memory", NULL);
		return NULL;
	}
	temporary->type.kind = kind;
	temporary->next = context->temporaryTypes;
	context->temporaryTypes = temporary;
	return &temporary->type;
}

static void _destroyTemporaryTypes(SemanticTemporaryType * type) {
	while (type != NULL) {
		SemanticTemporaryType * next = type->next;
		free(type);
		type = next;
	}
}

static SemanticExpressionInfo _expressionInfo(SemanticAnalysisContext * context, Expression * expression) {
	SemanticExpressionInfo info = { 0 };
	if (expression == NULL) {
		return info;
	}
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
			return _expressionInfoForIdentifier(context, expression);
		case EXPRESSION_INTEGER_LITERAL:
			return _expressionInfoWithType(&_semanticIntType, false);
		case EXPRESSION_FLOAT_LITERAL:
			return _expressionInfoWithType(&_semanticFloatType, false);
		case EXPRESSION_CHAR_LITERAL:
			return _expressionInfoWithType(&_semanticCharType, false);
		case EXPRESSION_FUNCTION_CALL:
			return _expressionInfoForFunctionCall(context, expression->functionCall);
		case EXPRESSION_BINARY_OPERATION:
			return _expressionInfoForBinaryOperation(context, expression);
		case EXPRESSION_UNARY_OPERATION:
			return _expressionInfoForUnaryOperation(context, expression);
		case EXPRESSION_ARRAY_LITERAL:
			_validateExpressionList(context, expression->elements);
			return info;
		case EXPRESSION_STRING_LITERAL:
		case EXPRESSION_NULL_LITERAL:
		default:
			return info;
	}
}

static SemanticExpressionInfo _expressionInfoForIdentifier(SemanticAnalysisContext * context, Expression * expression) {
	SemanticExpressionInfo info = { 0 };
	SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, expression->value);
	if (symbol == NULL) {
		_reportSemanticError(context, "Unknown identifier", expression->value);
		return info;
	}
	info.symbol = symbol;
	switch (symbol->kind) {
		case SEMANTIC_SYMBOL_VARIABLE:
			info.type = symbol->type;
			info.isLvalue = true;
			break;
		case SEMANTIC_SYMBOL_ENUM_CONSTANT:
			info.type = &_semanticIntType;
			break;
		case SEMANTIC_SYMBOL_FUNCTION:
			info.isFunctionDesignator = true;
			break;
		default:
			break;
	}
	return info;
}

static SemanticExpressionInfo _expressionInfoForFunctionCall(SemanticAnalysisContext * context, FunctionCall * functionCall) {
	SemanticExpressionInfo info = { 0 };
	if (functionCall == NULL) {
		return info;
	}
	SemanticSymbol * symbol = semanticSymbolTableLookupOrdinary(context->symbols, functionCall->name);
	if (!_isCallableSymbol(context, symbol)) {
		_reportSemanticError(context, "Unknown function", functionCall->name);
		_validateExpressionList(context, functionCall->arguments);
		return info;
	}
	_validateCallArguments(context, functionCall, symbol);
	info.type = _callableReturnType(context, symbol);
	return info;
}

static SemanticExpressionInfo _expressionInfoForBinaryOperation(SemanticAnalysisContext * context, Expression * expression) {
	SemanticExpressionInfo info = { 0 };
	if (_isAssignmentOperator(expression->operator)) {
		Type * targetType = _assignmentTargetType(context, expression->left);
		if (_isCompoundAssignmentOperator(expression->operator)) {
			SemanticExpressionInfo rightInfo = _expressionInfo(context, expression->right);
			if (targetType != NULL
				&& (!_isNumericScalarType(context, targetType)
					|| !_isNumericScalarType(context, rightInfo.type))) {
				_reportSemanticError(context, "Compound assignment requires numeric operands", NULL);
			}
		}
		else if (targetType != NULL && !_isExpressionAssignableToType(context, targetType, expression->right)) {
			_reportSemanticError(context, "Incompatible assignment", NULL);
		}
		info.type = targetType;
		return info;
	}
	if (expression->operator == EXPRESSION_OPERATOR_ARRAY_INDEX) {
		SemanticExpressionInfo leftInfo = _expressionInfo(context, expression->left);
		SemanticExpressionInfo rightInfo = _expressionInfo(context, expression->right);
		if (!_isNumericScalarType(context, rightInfo.type)) {
			_reportSemanticError(context, "Array index must be numeric", NULL);
		}
		info.type = _arrayIndexElementType(context, leftInfo.type);
		info.isLvalue = info.type != NULL;
		info.isReadonlyLvalue = info.isLvalue
			&& (leftInfo.isReadonlyLvalue || _effectiveConst(context, info.type));
		return info;
	}
	if (expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS
		|| expression->operator == EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS) {
		SemanticExpressionInfo leftInfo = _expressionInfo(context, expression->left);
		const char * fieldName = expression->right != NULL ? expression->right->value : NULL;
		if (expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS) {
			info.type = _memberAccessType(context, leftInfo.type, fieldName);
			info.isLvalue = leftInfo.isLvalue && info.type != NULL;
			info.isReadonlyLvalue = info.isLvalue
				&& (leftInfo.isReadonlyLvalue || _effectiveConst(context, leftInfo.type));
		}
		else {
			Type * receiverType = _resolveTypedef(context, leftInfo.type);
			if (receiverType == NULL || receiverType->kind != TYPE_POINTER_KIND) {
				_reportSemanticError(context, "Pointer member access requires pointer receiver", fieldName);
			}
			else {
				info.type = _memberAccessType(context, receiverType->pointee, fieldName);
				info.isLvalue = info.type != NULL;
				info.isReadonlyLvalue = info.isLvalue
					&& (leftInfo.isReadonlyLvalue || _effectiveConst(context, receiverType->pointee));
			}
		}
		return info;
	}

	SemanticExpressionInfo leftInfo = _expressionInfo(context, expression->left);
	SemanticExpressionInfo rightInfo = _expressionInfo(context, expression->right);
	if (_isNumericBinaryOperator(expression->operator)) {
		if (!_isNumericScalarType(context, leftInfo.type) || !_isNumericScalarType(context, rightInfo.type)) {
			_reportSemanticError(context, "Numeric operator requires numeric operands", NULL);
			return info;
		}
		info.type = leftInfo.type;
		return info;
	}
	if (_isEqualityOperator(expression->operator)) {
		if (!_isEqualityComparable(context, expression->left, leftInfo.type, expression->right, rightInfo.type)) {
			_reportSemanticError(context, "Comparison/logical operator requires numeric operands", NULL);
			return info;
		}
		info.type = &_semanticIntType;
		return info;
	}
	if (_isComparisonOperator(expression->operator) || _isLogicalOperator(expression->operator)) {
		if (!_isNumericScalarType(context, leftInfo.type) || !_isNumericScalarType(context, rightInfo.type)) {
			_reportSemanticError(context, "Comparison/logical operator requires numeric operands", NULL);
			return info;
		}
		info.type = &_semanticIntType;
		return info;
	}
	return info;
}

static SemanticExpressionInfo _expressionInfoForUnaryOperation(SemanticAnalysisContext * context, Expression * expression) {
	SemanticExpressionInfo info = { 0 };
	SemanticExpressionInfo operandInfo = _expressionInfo(context, expression->operand);
	switch (expression->operator) {
		case EXPRESSION_OPERATOR_DEREFERENCE: {
			Type * operandType = _resolveTypedef(context, operandInfo.type);
			if (operandType == NULL || operandType->kind != TYPE_POINTER_KIND) {
				_reportSemanticError(context, "Dereference requires pointer operand", NULL);
				return info;
			}
			info.type = operandType->pointee;
			info.isLvalue = true;
			info.isReadonlyLvalue = _effectiveConst(context, info.type);
			return info;
		}
		case EXPRESSION_OPERATOR_ADDRESS_OF:
			if (!operandInfo.isLvalue) {
				_reportSemanticError(context, "Address-of requires lvalue operand", NULL);
				return info;
			}
			if (operandInfo.type != NULL) {
				Type * pointerType = _createTemporaryType(context, TYPE_POINTER_KIND);
				if (pointerType != NULL) {
					pointerType->pointee = operandInfo.type;
					info.type = pointerType;
				}
			}
			return info;
		case EXPRESSION_OPERATOR_PREFIX_INCREMENT:
		case EXPRESSION_OPERATOR_PREFIX_DECREMENT:
		case EXPRESSION_OPERATOR_POSTFIX_INCREMENT:
		case EXPRESSION_OPERATOR_POSTFIX_DECREMENT:
			if (_assignmentTargetType(context, expression->operand) != NULL
				&& !_isNumericScalarType(context, operandInfo.type)) {
				_reportSemanticError(context, "Increment/decrement requires numeric operand", NULL);
			}
			info.type = operandInfo.type;
			return info;
		case EXPRESSION_OPERATOR_UNARY_PLUS:
		case EXPRESSION_OPERATOR_UNARY_MINUS:
		case EXPRESSION_OPERATOR_BITWISE_NOT:
			if (!_isNumericScalarType(context, operandInfo.type)) {
				_reportSemanticError(context, "Unary operator requires numeric operand", NULL);
				return info;
			}
			info.type = operandInfo.type;
			return info;
		case EXPRESSION_OPERATOR_LOGICAL_NOT:
			if (!_isNumericScalarType(context, operandInfo.type)) {
				_reportSemanticError(context, "Logical not requires numeric operand", NULL);
				return info;
			}
			info.type = &_semanticIntType;
			return info;
		default:
			return info;
	}
}

static SemanticExpressionInfo _expressionInfoWithType(Type * type, bool isLvalue) {
	SemanticExpressionInfo info = {
		.type = type,
		.isLvalue = isLvalue
	};
	return info;
}

static Type * _assignmentTargetType(SemanticAnalysisContext * context, Expression * expression) {
	SemanticExpressionInfo info = _expressionInfo(context, expression);
	if (!info.isLvalue) {
		_reportSemanticError(context, "Invalid assignment target", NULL);
		return NULL;
	}
	Type * targetType = _resolveTypedef(context, info.type);
	if (targetType == NULL) {
		return NULL;
	}
	if (targetType->kind == TYPE_ARRAY_KIND) {
		_reportSemanticError(context, "Cannot assign to array variable", NULL);
		return NULL;
	}
	if (info.isReadonlyLvalue || _effectiveConst(context, info.type)) {
		_reportSemanticError(context, "Cannot assign to const variable", NULL);
		return NULL;
	}
	return info.type;
}

static Type * _arrayIndexElementType(SemanticAnalysisContext * context, Type * indexedType) {
	Type * resolvedType = _resolveTypedef(context, indexedType);
	if (resolvedType == NULL) {
		return NULL;
	}
	if (resolvedType->kind == TYPE_ARRAY_KIND || resolvedType->kind == TYPE_POINTER_KIND) {
		return resolvedType->pointee;
	}
	_reportSemanticError(context, "Array index requires array or pointer receiver", NULL);
	return NULL;
}

static Type * _memberAccessType(SemanticAnalysisContext * context, Type * receiverType, const char * fieldName) {
	Type * resolvedType = _resolveTypedef(context, receiverType);
	if (resolvedType == NULL || fieldName == NULL) {
		return NULL;
	}
	if (resolvedType->kind != TYPE_STRUCT_KIND && resolvedType->kind != TYPE_UNION_KIND) {
		_reportSemanticError(context, "Member access requires aggregate receiver", fieldName);
		return NULL;
	}
	SemanticTagKind tagKind = resolvedType->kind == TYPE_STRUCT_KIND ? SEMANTIC_TAG_STRUCT : SEMANTIC_TAG_UNION;
	SemanticTag * tag = semanticSymbolTableLookupTag(context->symbols, resolvedType->name);
	AggregateDeclaration * aggregate = tag != NULL && tag->kind == tagKind ? tag->declaration : NULL;
	VariableDeclaration * field = _findAggregateField(aggregate, fieldName);
	if (field == NULL) {
		_reportSemanticError(context, "Unknown aggregate field", fieldName);
		return NULL;
	}
	return field->type;
}

static VariableDeclaration * _findAggregateField(AggregateDeclaration * declaration, const char * fieldName) {
	if (declaration == NULL || fieldName == NULL) {
		return NULL;
	}
	for (VariableDeclarationList * field = declaration->fields; field != NULL; field = field->next) {
		if (field->declaration != NULL && strcmp(field->declaration->name, fieldName) == 0) {
			return field->declaration;
		}
	}
	return NULL;
}

static bool _validateInitializer(SemanticAnalysisContext * context, VariableDeclaration * declaration) {
	if (declaration == NULL || declaration->initializer == NULL) {
		return true;
	}
	if (!_isExpressionAssignableToType(context, declaration->type, declaration->initializer)) {
		_reportSemanticError(context, "Incompatible initializer", declaration->name);
		return false;
	}
	return true;
}

static bool _isExpressionAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression) {
	if (targetType == NULL || expression == NULL) {
		return true;
	}
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	if (resolvedTargetType == NULL) {
		return true;
	}
	if (expression->kind == EXPRESSION_ARRAY_LITERAL) {
		return _isArrayLiteralAssignableToType(context, resolvedTargetType, expression);
	}
	if (resolvedTargetType->kind == TYPE_ARRAY_KIND) {
		SemanticExpressionInfo sourceInfo = _expressionInfo(context, expression);
		if (sourceInfo.type == NULL) {
			return true;
		}
		return _typesAssignable(context, targetType, sourceInfo.type);
	}
	if (_isNumericLiteral(expression)) {
		return _isNumericScalarType(context, targetType);
	}
	if (expression->kind == EXPRESSION_STRING_LITERAL) {
		return _isStringLiteralAssignableToType(context, targetType);
	}
	if (expression->kind == EXPRESSION_NULL_LITERAL) {
		return _isPointerLikeType(context, targetType);
	}
	if (expression->kind == EXPRESSION_UNARY_OPERATION
		&& expression->operator == EXPRESSION_OPERATOR_ADDRESS_OF) {
		return _isAddressExpressionAssignableToType(context, targetType, expression);
	}

	SemanticExpressionInfo sourceInfo = _expressionInfo(context, expression);
	if (sourceInfo.isFunctionDesignator) {
		return _isFunctionDesignatorAssignableToType(context, targetType, sourceInfo.symbol);
	}
	if (sourceInfo.type == NULL) {
		return true;
	}
	return _typesAssignable(context, targetType, sourceInfo.type);
}

static bool _isArrayLiteralAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression) {
	if (targetType == NULL || targetType->kind != TYPE_ARRAY_KIND) {
		_validateExpressionList(context, expression->elements);
		return false;
	}
	bool isAssignable = true;
	for (ExpressionList * item = expression->elements; item != NULL; item = item->next) {
		if (!_isExpressionAssignableToType(context, targetType->pointee, item->expression)) {
			isAssignable = false;
		}
	}
	size_t bound = 0;
	if (_fixedArrayBound(targetType->arraySize, &bound) && _expressionListLength(expression->elements) > bound) {
		isAssignable = false;
	}
	return isAssignable;
}

static bool _isAddressExpressionAssignableToType(SemanticAnalysisContext * context, Type * targetType, Expression * expression) {
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	if (resolvedTargetType == NULL || resolvedTargetType->kind != TYPE_POINTER_KIND) {
		(void) _expressionInfo(context, expression->operand);
		return false;
	}
	SemanticExpressionInfo operandInfo = _expressionInfo(context, expression->operand);
	if (!operandInfo.isLvalue || operandInfo.type == NULL) {
		return false;
	}
	return _pointeeTypesAssignable(context, resolvedTargetType->pointee, operandInfo.type);
}

static bool _isFunctionDesignatorAssignableToType(SemanticAnalysisContext * context, Type * targetType, SemanticSymbol * symbol) {
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	if (resolvedTargetType == NULL || resolvedTargetType->kind != TYPE_FUNCTION_POINTER_KIND) {
		return false;
	}
	return _callableSignatureMatches(context, symbol, resolvedTargetType->returnType, resolvedTargetType->functionParams);
}

static bool _isStringLiteralAssignableToType(SemanticAnalysisContext * context, Type * targetType) {
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	if (resolvedTargetType == NULL) {
		return true;
	}
	if (resolvedTargetType->kind == TYPE_CHAR_KIND) {
		return true;
	}
	if (resolvedTargetType->kind != TYPE_POINTER_KIND) {
		return false;
	}
	Type * pointee = _resolveTypedef(context, resolvedTargetType->pointee);
	return pointee != NULL && pointee->kind == TYPE_CHAR_KIND;
}

static bool _isNumericLiteral(Expression * expression) {
	return expression != NULL
		&& (expression->kind == EXPRESSION_INTEGER_LITERAL
			|| expression->kind == EXPRESSION_FLOAT_LITERAL
			|| expression->kind == EXPRESSION_CHAR_LITERAL);
}

static bool _isNumericScalarType(SemanticAnalysisContext * context, Type * type) {
	Type * resolvedType = _resolveTypedef(context, type);
	if (resolvedType == NULL) {
		return false;
	}
	switch (resolvedType->kind) {
		case TYPE_INT_KIND:
		case TYPE_CHAR_KIND:
		case TYPE_FLOAT_KIND:
		case TYPE_DOUBLE_KIND:
		case TYPE_UINT_KIND:
		case TYPE_ULI_KIND:
		case TYPE_LONG_KIND:
		case TYPE_SHORT_KIND:
		case TYPE_ENUM_KIND:
			return true;
		default:
			return false;
	}
}

static bool _isIntegerScalarType(SemanticAnalysisContext * context, Type * type) {
	Type * resolvedType = _resolveTypedef(context, type);
	if (resolvedType == NULL) {
		return false;
	}
	switch (resolvedType->kind) {
		case TYPE_INT_KIND:
		case TYPE_CHAR_KIND:
		case TYPE_UINT_KIND:
		case TYPE_ULI_KIND:
		case TYPE_LONG_KIND:
		case TYPE_SHORT_KIND:
		case TYPE_ENUM_KIND:
			return true;
		default:
			return false;
	}
}

static bool _isPointerLikeType(SemanticAnalysisContext * context, Type * type) {
	Type * resolvedType = _resolveTypedef(context, type);
	return resolvedType != NULL
		&& (resolvedType->kind == TYPE_POINTER_KIND || resolvedType->kind == TYPE_FUNCTION_POINTER_KIND);
}

static bool _isConditionType(SemanticAnalysisContext * context, Type * type) {
	return _isNumericScalarType(context, type) || _isPointerLikeType(context, type);
}

static bool _isVoidType(SemanticAnalysisContext * context, Type * type) {
	Type * resolvedType = _resolveTypedef(context, type);
	return resolvedType != NULL && resolvedType->kind == TYPE_VOID_KIND;
}

static bool _isDirectVoidType(SemanticAnalysisContext * context, Type * type) {
	return _isVoidType(context, type);
}

static bool _isNullLiteral(Expression * expression) {
	return expression != NULL && expression->kind == EXPRESSION_NULL_LITERAL;
}

static bool _isEqualityComparable(
	SemanticAnalysisContext * context,
	Expression * left,
	Type * leftType,
	Expression * right,
	Type * rightType) {
	if (_isNumericScalarType(context, leftType) && _isNumericScalarType(context, rightType)) {
		return true;
	}
	if (_isNullLiteral(left) && _isNullLiteral(right)) {
		return true;
	}
	if (_isNullLiteral(left)) {
		return _isPointerLikeType(context, rightType);
	}
	if (_isNullLiteral(right)) {
		return _isPointerLikeType(context, leftType);
	}
	return _isPointerLikeType(context, leftType)
		&& _isPointerLikeType(context, rightType)
		&& (_typesAssignable(context, leftType, rightType)
			|| _typesAssignable(context, rightType, leftType));
}

static bool _typesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType) {
	if (targetType == NULL || sourceType == NULL) {
		return targetType == sourceType;
	}
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	Type * resolvedSourceType = _resolveTypedef(context, sourceType);
	if (resolvedTargetType == NULL || resolvedSourceType == NULL) {
		return resolvedTargetType == resolvedSourceType;
	}
	if (_isNumericScalarType(context, resolvedTargetType)
		&& _isNumericScalarType(context, resolvedSourceType)) {
		return true;
	}
	if (resolvedTargetType->kind == TYPE_POINTER_KIND) {
		if (resolvedSourceType->kind == TYPE_ARRAY_KIND) {
			return _pointeeTypesAssignable(
				context,
				resolvedTargetType->pointee,
				resolvedSourceType->pointee);
		}
		if (resolvedSourceType->kind == TYPE_POINTER_KIND) {
			return _pointerTypesAssignable(context, resolvedTargetType, resolvedSourceType);
		}
		return false;
	}
	if (resolvedTargetType->kind == TYPE_ARRAY_KIND) {
		if (resolvedSourceType->kind == TYPE_POINTER_KIND) {
			return _pointeeTypesAssignable(
				context,
				resolvedTargetType->pointee,
				resolvedSourceType->pointee);
		}
		if (resolvedSourceType->kind == TYPE_ARRAY_KIND) {
			return _pointeeTypesAssignable(
					context,
					resolvedTargetType->pointee,
					resolvedSourceType->pointee)
				&& (resolvedTargetType->arraySize == NULL
					|| _expressionsEqual(resolvedTargetType->arraySize, resolvedSourceType->arraySize));
		}
		return false;
	}
	if (resolvedTargetType->kind != resolvedSourceType->kind) {
		return false;
	}
	switch (resolvedTargetType->kind) {
		case TYPE_NAMED_KIND:
		case TYPE_STRUCT_KIND:
		case TYPE_ENUM_KIND:
		case TYPE_UNION_KIND:
			if (resolvedTargetType->name == NULL || resolvedSourceType->name == NULL) {
				return resolvedTargetType->name == resolvedSourceType->name;
			}
			return strcmp(resolvedTargetType->name, resolvedSourceType->name) == 0;
		case TYPE_FUNCTION_POINTER_KIND:
			return _parameterTypesEqual(context, resolvedTargetType->functionParams, resolvedSourceType->functionParams)
				&& _typesEqual(context, resolvedTargetType->returnType, resolvedSourceType->returnType);
		default:
			return true;
	}
}

static bool _pointerTypesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType) {
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	Type * resolvedSourceType = _resolveTypedef(context, sourceType);
	if (resolvedTargetType == NULL
		|| resolvedSourceType == NULL
		|| resolvedTargetType->kind != TYPE_POINTER_KIND
		|| resolvedSourceType->kind != TYPE_POINTER_KIND) {
		return false;
	}
	return _pointeeTypesAssignable(
		context,
		resolvedTargetType->pointee,
		resolvedSourceType->pointee);
}

static bool _pointeeTypesAssignable(SemanticAnalysisContext * context, Type * targetType, Type * sourceType) {
	Type * resolvedTargetType = _resolveTypedef(context, targetType);
	Type * resolvedSourceType = _resolveTypedef(context, sourceType);
	if (resolvedTargetType == NULL || resolvedSourceType == NULL) {
		return resolvedTargetType == resolvedSourceType;
	}
	if (_effectiveConst(context, sourceType) && !_effectiveConst(context, targetType)) {
		return false;
	}
	if (resolvedTargetType->kind == TYPE_VOID_KIND || resolvedSourceType->kind == TYPE_VOID_KIND) {
		return true;
	}
	return _typesEqualIgnoringTopConst(context, resolvedTargetType, resolvedSourceType);
}

static bool _typesEqualIgnoringTopConst(SemanticAnalysisContext * context, Type * left, Type * right) {
	Type * resolvedLeft = _resolveTypedef(context, left);
	Type * resolvedRight = _resolveTypedef(context, right);
	if (resolvedLeft == NULL || resolvedRight == NULL) {
		return resolvedLeft == resolvedRight;
	}
	if (resolvedLeft->kind != resolvedRight->kind) {
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

static void _validateCallArguments(SemanticAnalysisContext * context, FunctionCall * functionCall, SemanticSymbol * symbol) {
	ParameterList * parameter = _callableParameters(context, symbol);
	ExpressionList * argument = functionCall != NULL ? functionCall->arguments : NULL;
	while (parameter != NULL && argument != NULL) {
		if (!_isExpressionAssignableToType(context, parameter->parameter->type, argument->expression)) {
			_reportSemanticError(context, "Incompatible function argument", functionCall->name);
		}
		parameter = parameter->next;
		argument = argument->next;
	}
	if (parameter != NULL || argument != NULL) {
		_reportSemanticError(context, "Function call argument count mismatch", functionCall->name);
	}
	while (argument != NULL) {
		_validateExpression(context, argument->expression);
		argument = argument->next;
	}
}

static size_t _expressionListLength(ExpressionList * expressionList) {
	size_t length = 0;
	for (ExpressionList * item = expressionList; item != NULL; item = item->next) {
		length++;
	}
	return length;
}

static bool _fixedArrayBound(Expression * expression, size_t * bound) {
	if (expression == NULL
		|| expression->kind != EXPRESSION_INTEGER_LITERAL
		|| expression->value == NULL
		|| bound == NULL) {
		return false;
	}
	return _parseArrayBoundLiteral(expression->value, bound);
}

static bool _parseArrayBoundLiteral(const char * literal, size_t * bound) {
	if (literal == NULL || bound == NULL) {
		return false;
	}
	int base = 10;
	const char * digits = literal;
	if (literal[0] == '0' && (literal[1] == 'x' || literal[1] == 'X')) {
		base = 16;
		digits = literal + 2;
	}
	else if (literal[0] == '0' && (literal[1] == 'o' || literal[1] == 'O')) {
		base = 8;
		digits = literal + 2;
	}
	if (*digits == '\0') {
		return false;
	}
	char * end = NULL;
	errno = 0;
	unsigned long long value = strtoull(digits, &end, base);
	if (end == digits || *end != '\0' || errno == ERANGE || value > (unsigned long long) SIZE_MAX) {
		return false;
	}
	*bound = (size_t) value;
	return true;
}

static bool _isNonPositiveArrayBound(Expression * expression) {
	if (expression == NULL) {
		return false;
	}
	if (expression->kind == EXPRESSION_UNARY_OPERATION
		&& expression->operator == EXPRESSION_OPERATOR_UNARY_MINUS
		&& expression->operand != NULL
		&& expression->operand->kind == EXPRESSION_INTEGER_LITERAL) {
		return true;
	}
	size_t bound = 0;
	return _fixedArrayBound(expression, &bound) && bound == 0;
}

static bool _isAssignmentOperator(ExpressionOperator operator) {
	return operator == EXPRESSION_OPERATOR_ASSIGN
		|| operator == EXPRESSION_OPERATOR_ADD_ASSIGN
		|| operator == EXPRESSION_OPERATOR_SUBTRACT_ASSIGN
		|| operator == EXPRESSION_OPERATOR_MULTIPLY_ASSIGN
		|| operator == EXPRESSION_OPERATOR_DIVIDE_ASSIGN
		|| operator == EXPRESSION_OPERATOR_MODULO_ASSIGN
		|| operator == EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN
		|| operator == EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN
		|| operator == EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN
		|| operator == EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN
		|| operator == EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN;
}

static bool _isCompoundAssignmentOperator(ExpressionOperator operator) {
	return _isAssignmentOperator(operator) && operator != EXPRESSION_OPERATOR_ASSIGN;
}

static bool _isNumericBinaryOperator(ExpressionOperator operator) {
	return operator == EXPRESSION_OPERATOR_BITWISE_OR
		|| operator == EXPRESSION_OPERATOR_BITWISE_XOR
		|| operator == EXPRESSION_OPERATOR_BITWISE_AND
		|| operator == EXPRESSION_OPERATOR_SHIFT_LEFT
		|| operator == EXPRESSION_OPERATOR_SHIFT_RIGHT
		|| operator == EXPRESSION_OPERATOR_ADD
		|| operator == EXPRESSION_OPERATOR_SUBTRACT
		|| operator == EXPRESSION_OPERATOR_MULTIPLY
		|| operator == EXPRESSION_OPERATOR_DIVIDE
		|| operator == EXPRESSION_OPERATOR_MODULO;
}

static bool _isEqualityOperator(ExpressionOperator operator) {
	return operator == EXPRESSION_OPERATOR_EQUAL
		|| operator == EXPRESSION_OPERATOR_NOT_EQUAL;
}

static bool _isComparisonOperator(ExpressionOperator operator) {
	return operator == EXPRESSION_OPERATOR_EQUAL
		|| operator == EXPRESSION_OPERATOR_NOT_EQUAL
		|| operator == EXPRESSION_OPERATOR_LESS_THAN
		|| operator == EXPRESSION_OPERATOR_GREATER_THAN
		|| operator == EXPRESSION_OPERATOR_LESS_EQUAL
		|| operator == EXPRESSION_OPERATOR_GREATER_EQUAL;
}

static bool _isLogicalOperator(ExpressionOperator operator) {
	return operator == EXPRESSION_OPERATOR_LOGICAL_OR
		|| operator == EXPRESSION_OPERATOR_LOGICAL_AND;
}

static bool _functionSignaturesEqual(SemanticAnalysisContext * context, SemanticSymbol * symbol, FunctionDeclaration * declaration) {
	return _callableSignatureMatches(context, symbol, declaration->returnType, declaration->parameters);
}

static bool _callableSignatureMatches(
	SemanticAnalysisContext * context,
	SemanticSymbol * symbol,
	Type * returnType,
	ParameterList * parameters) {
	if (!_isCallableSymbol(context, symbol)) {
		return false;
	}
	return _typesEqual(context, _callableReturnType(context, symbol), returnType)
		&& _parameterTypesEqual(context, _callableParameters(context, symbol), parameters);
}

static Type * _callableReturnType(SemanticAnalysisContext * context, SemanticSymbol * symbol) {
	if (symbol == NULL) {
		return NULL;
	}
	if (symbol->kind == SEMANTIC_SYMBOL_FUNCTION) {
		return symbol->returnType;
	}
	if (symbol->kind != SEMANTIC_SYMBOL_VARIABLE) {
		return NULL;
	}
	Type * type = _resolveTypedef(context, symbol->type);
	if (type == NULL || type->kind != TYPE_FUNCTION_POINTER_KIND) {
		return NULL;
	}
	return type->returnType;
}

static ParameterList * _callableParameters(SemanticAnalysisContext * context, SemanticSymbol * symbol) {
	if (symbol == NULL) {
		return NULL;
	}
	if (symbol->kind == SEMANTIC_SYMBOL_FUNCTION) {
		return symbol->parameters;
	}
	if (symbol->kind != SEMANTIC_SYMBOL_VARIABLE) {
		return NULL;
	}
	Type * type = _resolveTypedef(context, symbol->type);
	if (type == NULL || type->kind != TYPE_FUNCTION_POINTER_KIND) {
		return NULL;
	}
	return type->functionParams;
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
	return _callableReturnType(context, symbol) != NULL;
}

static const char * _declaredLoopIteratorName(ForStatement * statement) {
	if (statement == NULL
		|| !statement->declaresIterator
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
		.temporaryTypes = NULL,
		.hasErrors = false
	};
	semanticSymbolTablePushScope(context.symbols);

	Program * program = (Program *) compilerState->abstractSyntaxtTree;
	_collectGlobalSymbols(&context, program);
	_validateProgram(&context, program);

	bool hasErrors = context.hasErrors;
	destroySemanticSymbolTable(context.symbols);
	_destroyTemporaryTypes(context.temporaryTypes);
	return hasErrors ? FAILED : SUCCEEDED;
}
