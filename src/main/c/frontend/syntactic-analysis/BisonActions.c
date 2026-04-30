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

Type * IntTypeSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPE_INT_KIND;
	return type;
}

VariableDeclaration * VariableDeclarationSemanticAction(char * name, Type * type) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclaration * declaration = calloc(1, sizeof(VariableDeclaration));
	declaration->name = name;
	declaration->type = type;
	return declaration;
}

VariableDeclarationList * SingletonDeclarationListSemanticAction(VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclarationList * declarationList = calloc(1, sizeof(VariableDeclarationList));
	declarationList->declaration = declaration;
	return declarationList;
}

VariableDeclarationList * AppendDeclarationListSemanticAction(VariableDeclarationList * declarationList, VariableDeclaration * declaration) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	VariableDeclarationList * tail = declarationList;
	while (tail->next != NULL) {
		tail = tail->next;
	}
	tail->next = SingletonDeclarationListSemanticAction(declaration);
	return declarationList;
}

Program * ProgramSemanticAction(VariableDeclarationList * declarations) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->declarations = declarations;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}
