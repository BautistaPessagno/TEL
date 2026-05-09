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
		if (type->name != NULL) {
			free(type->name);
			type->name = NULL;
		}
		destroyType(type->pointee);
		type->pointee = NULL;
		destroyExpression(type->arraySize);
		type->arraySize = NULL;
		destroyParameterList(type->functionParams);
		type->functionParams = NULL;
		destroyType(type->returnType);
		type->returnType = NULL;
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
		destroyExpression(variableDeclaration->initializer);
		variableDeclaration->initializer = NULL;
		free(variableDeclaration);
	}
}

void destroyVariableDeclarationList(VariableDeclarationList * variableDeclarationList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (variableDeclarationList != NULL) {
		destroyVariableDeclaration(variableDeclarationList->declaration);
		variableDeclarationList->declaration = NULL;
		destroyVariableDeclarationList(variableDeclarationList->next);
		variableDeclarationList->next = NULL;
		free(variableDeclarationList);
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
		destroyStatementList(functionDeclaration->body);
		functionDeclaration->body = NULL;
		free(functionDeclaration);
	}
}

void destroyMainDeclaration(MainDeclaration * mainDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (mainDeclaration != NULL) {
		destroyStatementList(mainDeclaration->body);
		mainDeclaration->body = NULL;
		free(mainDeclaration);
	}
}

void destroyAggregateDeclaration(AggregateDeclaration * aggregateDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (aggregateDeclaration != NULL) {
		if (aggregateDeclaration->name != NULL) {
			free(aggregateDeclaration->name);
			aggregateDeclaration->name = NULL;
		}
		destroyVariableDeclarationList(aggregateDeclaration->fields);
		aggregateDeclaration->fields = NULL;
		free(aggregateDeclaration);
	}
}

void destroyEnumMember(EnumMember * enumMember) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (enumMember != NULL) {
		if (enumMember->name != NULL) {
			free(enumMember->name);
			enumMember->name = NULL;
		}
		if (enumMember->value != NULL) {
			free(enumMember->value);
			enumMember->value = NULL;
		}
		free(enumMember);
	}
}

void destroyEnumMemberList(EnumMemberList * enumMemberList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (enumMemberList != NULL) {
		destroyEnumMember(enumMemberList->member);
		enumMemberList->member = NULL;
		destroyEnumMemberList(enumMemberList->next);
		enumMemberList->next = NULL;
		free(enumMemberList);
	}
}

void destroyEnumDeclaration(EnumDeclaration * enumDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (enumDeclaration != NULL) {
		if (enumDeclaration->name != NULL) {
			free(enumDeclaration->name);
			enumDeclaration->name = NULL;
		}
		destroyEnumMemberList(enumDeclaration->members);
		enumDeclaration->members = NULL;
		free(enumDeclaration);
	}
}

void destroyTypedefDeclaration(TypedefDeclaration * typedefDeclaration) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (typedefDeclaration != NULL) {
		if (typedefDeclaration->name != NULL) {
			free(typedefDeclaration->name);
			typedefDeclaration->name = NULL;
		}
		destroyType(typedefDeclaration->type);
		typedefDeclaration->type = NULL;
		free(typedefDeclaration);
	}
}

void destroyPreprocessorDirective(PreprocessorDirective * preprocessorDirective) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (preprocessorDirective != NULL) {
		if (preprocessorDirective->value != NULL) {
			free(preprocessorDirective->value);
			preprocessorDirective->value = NULL;
		}
		free(preprocessorDirective);
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
		destroyExpression(expression->operand);
		expression->operand = NULL;
		destroyExpression(expression->left);
		expression->left = NULL;
		destroyExpression(expression->right);
		expression->right = NULL;
		destroyExpressionList(expression->elements);
		expression->elements = NULL;
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

void destroyProgramItem(ProgramItem * programItem) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (programItem != NULL) {
		destroyVariableDeclaration(programItem->variableDeclaration);
		programItem->variableDeclaration = NULL;
		destroyFunctionDeclaration(programItem->functionDeclaration);
		programItem->functionDeclaration = NULL;
		destroyMainDeclaration(programItem->mainDeclaration);
		programItem->mainDeclaration = NULL;
		destroyAggregateDeclaration(programItem->aggregateDeclaration);
		programItem->aggregateDeclaration = NULL;
		destroyEnumDeclaration(programItem->enumDeclaration);
		programItem->enumDeclaration = NULL;
		destroyTypedefDeclaration(programItem->typedefDeclaration);
		programItem->typedefDeclaration = NULL;
		destroyPreprocessorDirective(programItem->preprocessorDirective);
		programItem->preprocessorDirective = NULL;
		free(programItem);
	}
}

void destroyProgramItemList(ProgramItemList * programItemList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (programItemList != NULL) {
		destroyProgramItem(programItemList->item);
		programItemList->item = NULL;
		destroyProgramItemList(programItemList->next);
		programItemList->next = NULL;
		free(programItemList);
	}
}

void destroyIfBranch(IfBranch * ifBranch) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (ifBranch != NULL) {
		destroyExpression(ifBranch->condition);
		ifBranch->condition = NULL;
		destroyStatementList(ifBranch->body);
		ifBranch->body = NULL;
		destroyIfBranch(ifBranch->next);
		ifBranch->next = NULL;
		free(ifBranch);
	}
}

void destroyIfStatement(IfStatement * ifStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (ifStatement != NULL) {
		destroyIfBranch(ifStatement->branches);
		ifStatement->branches = NULL;
		destroyStatementList(ifStatement->elseBody);
		ifStatement->elseBody = NULL;
		free(ifStatement);
	}
}

void destroyForStatement(ForStatement * forStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (forStatement != NULL) {
		destroyExpression(forStatement->initializer);
		forStatement->initializer = NULL;
		destroyExpression(forStatement->condition);
		forStatement->condition = NULL;
		destroyExpression(forStatement->update);
		forStatement->update = NULL;
		destroyStatementList(forStatement->body);
		forStatement->body = NULL;
		free(forStatement);
	}
}

void destroyWhileStatement(WhileStatement * whileStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (whileStatement != NULL) {
		destroyExpression(whileStatement->condition);
		whileStatement->condition = NULL;
		destroyStatementList(whileStatement->body);
		whileStatement->body = NULL;
		free(whileStatement);
	}
}

void destroyDoWhileStatement(DoWhileStatement * doWhileStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (doWhileStatement != NULL) {
		destroyStatementList(doWhileStatement->body);
		doWhileStatement->body = NULL;
		destroyExpression(doWhileStatement->condition);
		doWhileStatement->condition = NULL;
		free(doWhileStatement);
	}
}

void destroySwitchCase(SwitchCase * switchCase) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (switchCase != NULL) {
		destroyExpression(switchCase->matchExpression);
		switchCase->matchExpression = NULL;
		destroyStatementList(switchCase->body);
		switchCase->body = NULL;
		destroySwitchCase(switchCase->next);
		switchCase->next = NULL;
		free(switchCase);
	}
}

void destroySwitchStatement(SwitchStatement * switchStatement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (switchStatement != NULL) {
		destroyExpression(switchStatement->discriminant);
		switchStatement->discriminant = NULL;
		destroySwitchCase(switchStatement->cases);
		switchStatement->cases = NULL;
		free(switchStatement);
	}
}

void destroyStatement(Statement * statement) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statement != NULL) {
		destroyVariableDeclaration(statement->variableDeclaration);
		statement->variableDeclaration = NULL;
		destroyExpression(statement->expression);
		statement->expression = NULL;
		destroyIfStatement(statement->ifStatement);
		statement->ifStatement = NULL;
		destroyForStatement(statement->forStatement);
		statement->forStatement = NULL;
		destroyWhileStatement(statement->whileStatement);
		statement->whileStatement = NULL;
		destroyDoWhileStatement(statement->doWhileStatement);
		statement->doWhileStatement = NULL;
		destroySwitchStatement(statement->switchStatement);
		statement->switchStatement = NULL;
		free(statement);
	}
}

void destroyStatementList(StatementList * statementList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (statementList != NULL) {
		destroyStatement(statementList->statement);
		statementList->statement = NULL;
		destroyStatementList(statementList->next);
		statementList->next = NULL;
		free(statementList);
	}
}

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyProgramItemList(program->items);
		program->items = NULL;
		free(program);
	}
}
