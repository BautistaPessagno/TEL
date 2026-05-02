#include "FlexActions.h"
#include "StackADT.h"
#include <string.h>

/* MODULE INTERNAL STATE */

#define INDENTATION_STACK_CAPACITY 128

static StackADT * _indentationStack = NULL;
static bool _logIgnoredLexemes = true;
static LexicalAnalyzer * _lexicalAnalyzer = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownFlexActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: FlexActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	destroyStackADT(_indentationStack);
	_indentationStack = NULL;
	_lexicalAnalyzer = NULL;
}

ModuleDestructor initializeFlexActionsModule(LexicalAnalyzer * lexicalAnalyzer) {
	_lexicalAnalyzer = lexicalAnalyzer;
	_logger = createLogger("FlexActions");
	_logIgnoredLexemes = getBooleanOrDefault("LOG_IGNORED_LEXEMES", _logIgnoredLexemes);
	_indentationStack = createStackADT(INDENTATION_STACK_CAPACITY);
	if (_indentationStack != NULL) {
		pushStackADT(_indentationStack, 0);
	}
	return _shutdownFlexActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logTokenAction(const char * actionName, Token * token);
static CompilationStatus _pushToken(TokenLabel label, const char * actionName);
static CompilationStatus _pushDedentsUntil(unsigned int indentation);

/**
 * Logs a lexical-analyzer action over a token in DEBUGGING level.
 */
static void _logTokenAction(const char * actionName, Token * token) {
	char * _lexeme = escape(token->lexeme);
	logDebugging(_logger, WARNING_COLOR "%s" DEFAULT_COLOR ": Token(context=%d, label=%d, length=%d, lexeme=%s\"%s\"%s, line=%d, semanticValue=%p)",
		actionName,
		token->context,
		token->label,
		token->length,
		INFORMATION_COLOR, _lexeme, DEFAULT_COLOR,
		token->line,
		token->semanticValue);
	free(_lexeme);
	_lexeme = NULL;
}

static CompilationStatus _pushToken(TokenLabel label, const char * actionName) {
	Token * token = createToken(_lexicalAnalyzer, label);
	_logTokenAction(actionName, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

static CompilationStatus _pushDedentsUntil(unsigned int indentation) {
	unsigned int currentIndentation = 0;
	if (!peekStackADT(_indentationStack, &currentIndentation)) {
		return OUT_OF_MEMORY;
	}
	while (sizeStackADT(_indentationStack) > 1 && currentIndentation > indentation) {
		popStackADT(_indentationStack, &currentIndentation);
		CompilationStatus status = _pushToken(DEDENT, "DedentationLexemeAction");
		if (status != IN_PROGRESS) {
			return status;
		}
		if (!peekStackADT(_indentationStack, &currentIndentation)) {
			return OUT_OF_MEMORY;
		}
	}
	if (currentIndentation != indentation) {
		return FAILED;
	}
	return IN_PROGRESS;
}

/* PUBLIC FUNCTIONS */

CompilationStatus IdentifierLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, IDENTIFIER);
	token->semanticValue->string = strdup(token->lexeme);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus StringLexemeAction(TokenLabel label) {
	Token * token = createToken(_lexicalAnalyzer, label);
	token->semanticValue->string = strdup(token->lexeme);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus KeywordLexemeAction(TokenLabel label) {
	Token * token = createToken(_lexicalAnalyzer, label);
	_logTokenAction(__FUNCTION__, token);
	CompilationStatus status = pushToken(_lexicalAnalyzer, token);
	destroyToken(token);
	return status;
}

CompilationStatus PunctuationLexemeAction(TokenLabel label) {
	return _pushToken(label, __FUNCTION__);
}

CompilationStatus IndentationLexemeAction() {
	unsigned int indentation = 0;
	const char * lexeme = yyget_text(_lexicalAnalyzer->scanner);
	while (*lexeme == ' ' || *lexeme == '\t') {
		indentation += (*lexeme == '\t') ? 4 : 1;
		lexeme++;
	}
	if (indentation % 4 != 0) {
		return FAILED;
	}

	unsigned int currentIndentation = 0;
	if (!peekStackADT(_indentationStack, &currentIndentation)) {
		return OUT_OF_MEMORY;
	}
	if (indentation > currentIndentation) {
		if (!pushStackADT(_indentationStack, indentation)) {
			return OUT_OF_MEMORY;
		}
		return _pushToken(INDENT, __FUNCTION__);
	}
	if (indentation < currentIndentation) {
		return _pushDedentsUntil(indentation);
	}
	return IN_PROGRESS;
}

CompilationStatus LineBreakLexemeAction() {
	return _pushToken(SEMICOLON, __FUNCTION__);
}

CompilationStatus TerminatorLexemeAction() {
	return _pushToken(SEMICOLON, __FUNCTION__);
}

CompilationStatus EOFLexemeAction() {
	CompilationStatus status = TerminatorLexemeAction();
	if (status != IN_PROGRESS) {
		return status;
	}
	status = _pushDedentsUntil(0);
	if (status != IN_PROGRESS) {
		return status;
	}
	return _pushToken(0, __FUNCTION__);
}

CompilationStatus IgnoredLexemeAction() {
	if (_logIgnoredLexemes) {
		Token * token = createToken(_lexicalAnalyzer, IGNORED);
		_logTokenAction(__FUNCTION__, token);
		destroyToken(token);
	}
	return IN_PROGRESS;
}

CompilationStatus UnknownLexemeAction() {
	Token * token = createToken(_lexicalAnalyzer, UNKNOWN);
	_logTokenAction(__FUNCTION__, token);
	destroyToken(token);
	return FAILED;
}
