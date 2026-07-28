#include "CDeclarator.h"
#include "../../syntactic-analysis/AstBuilder.h"
#include <stdlib.h>
#include <string.h>

static Expression * _cloneExpression(Expression * expression);

static ExpressionList * _cloneExpressionList(ExpressionList * expressions) {
	ExpressionList * result = NULL;
	for (ExpressionList * node = expressions; node != NULL; node = node->next) {
		result = AppendExpressionListSemanticAction(
			result,
			_cloneExpression(node->expression));
	}
	return result;
}

static ParameterList * _cloneParameters(ParameterList * parameters) {
	ParameterList * result = NULL;
	for (ParameterList * node = parameters; node != NULL; node = node->next) {
		Parameter * parameter = node->parameter;
		result = AppendParameterListSemanticAction(
			result,
			ParameterSemanticAction(
				parameter->name != NULL ? strdup(parameter->name) : NULL,
				cloneCType(parameter->type)));
	}
	return result;
}

static Expression * _cloneExpression(Expression * expression) {
	if (expression == NULL) {
		return NULL;
	}
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
			return IdentifierExpressionSemanticAction(strdup(expression->value));
		case EXPRESSION_INTEGER_LITERAL:
			return IntegerLiteralExpressionSemanticAction(strdup(expression->value));
		case EXPRESSION_FLOAT_LITERAL:
			return FloatLiteralExpressionSemanticAction(strdup(expression->value));
		case EXPRESSION_CHAR_LITERAL:
			return CharLiteralExpressionSemanticAction(strdup(expression->value));
		case EXPRESSION_STRING_LITERAL:
			return StringLiteralExpressionSemanticAction(strdup(expression->value));
		case EXPRESSION_NULL_LITERAL:
			return NullLiteralExpressionSemanticAction();
		case EXPRESSION_ARRAY_LITERAL:
			return ArrayLiteralExpressionSemanticAction(
				_cloneExpressionList(expression->elements));
		case EXPRESSION_FUNCTION_CALL:
			return FunctionCallExpressionSemanticAction(
				FunctionCallSemanticAction(
					strdup(expression->functionCall->name),
					_cloneExpressionList(expression->functionCall->arguments)));
		case EXPRESSION_BINARY_OPERATION:
			return BinaryExpressionSemanticAction(
				_cloneExpression(expression->left),
				expression->operator,
				_cloneExpression(expression->right));
		case EXPRESSION_UNARY_OPERATION:
			return UnaryExpressionSemanticAction(
				expression->operator,
				_cloneExpression(expression->operand));
	}
	return NULL;
}

Type * cloneCType(Type * type) {
	if (type == NULL) {
		return NULL;
	}
	Type * copy = calloc(1, sizeof(Type));
	copy->kind = type->kind;
	copy->name = type->name != NULL ? strdup(type->name) : NULL;
	copy->pointee = cloneCType(type->pointee);
	copy->arraySize = _cloneExpression(type->arraySize);
	copy->functionParams = _cloneParameters(type->functionParams);
	copy->returnType = cloneCType(type->returnType);
	copy->isConst = type->isConst;
	return copy;
}

CDeclarator * createCIdentifierDeclarator(char * name) {
	CDeclarator * declarator = calloc(1, sizeof(CDeclarator));
	declarator->kind = C_DECLARATOR_IDENTIFIER;
	declarator->name = name;
	return declarator;
}

CDeclarator * createCPointerDeclarator(CDeclarator * inner) {
	CDeclarator * declarator = calloc(1, sizeof(CDeclarator));
	declarator->kind = C_DECLARATOR_POINTER;
	declarator->inner = inner;
	return declarator;
}

CDeclarator * createCArrayDeclarator(CDeclarator * inner, Expression * size) {
	CDeclarator * declarator = calloc(1, sizeof(CDeclarator));
	declarator->kind = C_DECLARATOR_ARRAY;
	declarator->inner = inner;
	declarator->arraySize = size;
	return declarator;
}

CDeclarator * createCFunctionDeclarator(CDeclarator * inner, ParameterList * parameters) {
	if (parameters != NULL
		&& parameters->next == NULL
		&& parameters->parameter != NULL
		&& parameters->parameter->name == NULL
		&& parameters->parameter->type != NULL
		&& parameters->parameter->type->kind == TYPE_VOID_KIND) {
		destroyParameterList(parameters);
		parameters = NULL;
	}
	CDeclarator * declarator = calloc(1, sizeof(CDeclarator));
	declarator->kind = C_DECLARATOR_FUNCTION;
	declarator->inner = inner;
	declarator->parameters = parameters;
	return declarator;
}

CDeclarator * setCDeclaratorInitializer(CDeclarator * declarator, Expression * initializer) {
	declarator->initializer = initializer;
	return declarator;
}

const char * cDeclaratorName(CDeclarator * declarator) {
	for (CDeclarator * node = declarator; node != NULL; node = node->inner) {
		if (node->kind == C_DECLARATOR_IDENTIFIER) {
			return node->name;
		}
	}
	return NULL;
}

CDeclaratorList * appendCDeclarator(CDeclaratorList * list, CDeclarator * declarator) {
	CDeclaratorList * node = calloc(1, sizeof(CDeclaratorList));
	node->declarator = declarator;
	if (list == NULL) {
		return node;
	}
	CDeclaratorList * tail = list;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = node;
	return list;
}

CDeclarationSpecifiers * createCDeclarationSpecifiers(
	Type * type,
	bool isStatic,
	bool isTypedef) {
	CDeclarationSpecifiers * specifiers = calloc(1, sizeof(CDeclarationSpecifiers));
	specifiers->type = type;
	specifiers->isStatic = isStatic;
	specifiers->isTypedef = isTypedef;
	return specifiers;
}

static CResolvedDeclarator * _resolveDeclarator(
	CDeclarator * declarator,
	Type * baseType,
	Expression * initializer,
	bool consumeFunctionPointer) {
	if (declarator == NULL) {
		destroyType(baseType);
		destroyExpression(initializer);
		return NULL;
	}
	if (declarator->kind == C_DECLARATOR_IDENTIFIER) {
		CResolvedDeclarator * resolved = calloc(1, sizeof(CResolvedDeclarator));
		resolved->name = declarator->name;
		declarator->name = NULL;
		resolved->type = baseType;
		resolved->initializer = initializer;
		return resolved;
	}
	if (declarator->kind == C_DECLARATOR_POINTER) {
		Type * type = baseType;
		bool consumed = consumeFunctionPointer && baseType->kind == TYPE_FUNCTION_POINTER_KIND;
		if (!consumed) {
			type = PointerTypeSemanticAction(baseType);
		}
		return _resolveDeclarator(declarator->inner, type, initializer, false);
	}
	if (declarator->kind == C_DECLARATOR_ARRAY) {
		Expression * size = declarator->arraySize;
		declarator->arraySize = NULL;
		return _resolveDeclarator(
			declarator->inner,
			ArrayTypeSemanticAction(baseType, size),
			initializer,
			consumeFunctionPointer);
	}
	if (declarator->inner != NULL
		&& declarator->inner->kind == C_DECLARATOR_IDENTIFIER) {
		CResolvedDeclarator * resolved = calloc(1, sizeof(CResolvedDeclarator));
		resolved->name = declarator->inner->name;
		declarator->inner->name = NULL;
		resolved->parameters = declarator->parameters;
		declarator->parameters = NULL;
		resolved->returnType = baseType;
		resolved->initializer = initializer;
		resolved->isFunction = true;
		return resolved;
	}
	ParameterList * parameters = declarator->parameters;
	declarator->parameters = NULL;
	return _resolveDeclarator(
		declarator->inner,
		FunctionPointerTypeSemanticAction(parameters, baseType),
		initializer,
		true);
}

CResolvedDeclarator * resolveCDeclarator(CDeclarator * declarator, Type * baseType) {
	Expression * initializer = declarator != NULL ? declarator->initializer : NULL;
	if (declarator != NULL) {
		declarator->initializer = NULL;
	}
	CResolvedDeclarator * resolved = _resolveDeclarator(
		declarator,
		baseType,
		initializer,
		false);
	destroyCDeclarator(declarator);
	return resolved;
}

void destroyCDeclarator(CDeclarator * declarator) {
	if (declarator == NULL) {
		return;
	}
	free(declarator->name);
	destroyCDeclarator(declarator->inner);
	destroyExpression(declarator->arraySize);
	destroyParameterList(declarator->parameters);
	destroyExpression(declarator->initializer);
	free(declarator);
}

void destroyCDeclaratorList(CDeclaratorList * list) {
	while (list != NULL) {
		CDeclaratorList * next = list->next;
		destroyCDeclarator(list->declarator);
		free(list);
		list = next;
	}
}

void destroyCResolvedDeclarator(CResolvedDeclarator * declarator) {
	if (declarator == NULL) {
		return;
	}
	free(declarator->name);
	destroyType(declarator->type);
	destroyParameterList(declarator->parameters);
	destroyType(declarator->returnType);
	destroyExpression(declarator->initializer);
	free(declarator);
}

void destroyCDeclarationSpecifiers(CDeclarationSpecifiers * specifiers) {
	if (specifiers == NULL) {
		return;
	}
	destroyType(specifiers->type);
	free(specifiers);
}
