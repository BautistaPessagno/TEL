#include "CodeGenerator.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownCodeGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: CodeGenerator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCodeGeneratorModule() {
	_logger = createLogger("CodeGenerator");
	return _shutdownCodeGeneratorModule;
}

/** PUBLIC FUNCTIONS */

CompilationStatus executeCodeGeneration(CompilerState * compilerState) {
	logDebugging(_logger, "Executing code generation...");
	(void) compilerState;
	return SUCCEEDED;
}
