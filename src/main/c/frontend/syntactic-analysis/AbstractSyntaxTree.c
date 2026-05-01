#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyType(Type * type) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (type != NULL) {
		free(type);
	}
}

void destroyVariableDeclaration(VariableDeclaration * variableDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (variableDeclaration != NULL) {
		if (variableDeclaration->name != NULL) {
			free(variableDeclaration->name);
			variableDeclaration->name = NULL;
		}
		destroyType(variableDeclaration->type);
		variableDeclaration->type = NULL;
		free(variableDeclaration);
	}
}

void destroyParameter(Parameter * parameter) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (parameter != NULL) {
		if (parameter->name != NULL) {
			free(parameter->name);
			parameter->name = NULL;
		}
		destroyType(parameter->type);
		parameter->type = NULL;
		free(parameter);
	}
}

void destroyParameterList(ParameterList * parameterList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (parameterList != NULL) {
		destroyParameter(parameterList->parameter);
		parameterList->parameter = NULL;
		destroyParameterList(parameterList->next);
		parameterList->next = NULL;
		free(parameterList);
	}
}

void destroyFunctionDeclaration(FunctionDeclaration * functionDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (functionDeclaration != NULL) {
		if (functionDeclaration->name != NULL) {
			free(functionDeclaration->name);
			functionDeclaration->name = NULL;
		}
		destroyParameterList(functionDeclaration->parameters);
		functionDeclaration->parameters = NULL;
		destroyType(functionDeclaration->returnType);
		functionDeclaration->returnType = NULL;
		free(functionDeclaration);
	}
}

void destroyFunctionCall(FunctionCall * functionCall) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (functionCall != NULL) {
		if (functionCall->name != NULL) {
			free(functionCall->name);
			functionCall->name = NULL;
		}
		destroyExpressionList(functionCall->arguments);
		functionCall->arguments = NULL;
		free(functionCall);
	}
}

void destroyExpression(Expression * expression) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expression != NULL) {
		if (expression->value != NULL) {
			free(expression->value);
			expression->value = NULL;
		}
		destroyFunctionCall(expression->functionCall);
		expression->functionCall = NULL;
		free(expression);
	}
}

void destroyExpressionList(ExpressionList * expressionList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expressionList != NULL) {
		destroyExpression(expressionList->expression);
		expressionList->expression = NULL;
		destroyExpressionList(expressionList->next);
		expressionList->next = NULL;
		free(expressionList);
	}
}

void destroyTopLevelItem(TopLevelItem * topLevelItem) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (topLevelItem != NULL) {
		destroyVariableDeclaration(topLevelItem->variableDeclaration);
		topLevelItem->variableDeclaration = NULL;
		destroyFunctionDeclaration(topLevelItem->functionDeclaration);
		topLevelItem->functionDeclaration = NULL;
		destroyFunctionCall(topLevelItem->functionCall);
		topLevelItem->functionCall = NULL;
		free(topLevelItem);
	}
}

void destroyTopLevelItemList(TopLevelItemList * topLevelItemList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (topLevelItemList != NULL) {
		destroyTopLevelItem(topLevelItemList->item);
		topLevelItemList->item = NULL;
		destroyTopLevelItemList(topLevelItemList->next);
		topLevelItemList->next = NULL;
		free(topLevelItemList);
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyTopLevelItemList(program->items);
		program->items = NULL;
		free(program);
	}
}
