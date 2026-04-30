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

void destroyProgram(Program * program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroyVariableDeclarationList(program->declarations);
		program->declarations = NULL;
		free(program);
	}
}
