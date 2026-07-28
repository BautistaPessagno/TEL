#include "CFrontend.h"
#include "lexical-analysis/CScanner.h"
#include "syntactic-analysis/CParser.h"
#include <stdlib.h>
#include <string.h>

struct CIdentifierBinding {
	char * name;
	unsigned int scopeDepth;
	bool isTypedef;
	CIdentifierBinding * next;
};

bool isCFrontendTypedefName(CFrontendContext * context, const char * name) {
	if (context == NULL || name == NULL) {
		return false;
	}
	for (CIdentifierBinding * node = context->identifierBindings;
		node != NULL;
		node = node->next) {
		if (strcmp(node->name, name) == 0) {
			return node->isTypedef;
		}
	}
	return false;
}

static void _registerBinding(
	CFrontendContext * context,
	const char * name,
	bool isTypedef) {
	if (context == NULL || name == NULL) {
		return;
	}
	CIdentifierBinding * node = calloc(1, sizeof(CIdentifierBinding));
	if (node == NULL) {
		return;
	}
	node->name = strdup(name);
	if (node->name == NULL) {
		free(node);
		return;
	}
	node->scopeDepth = context->scopeDepth;
	node->isTypedef = isTypedef;
	node->next = context->identifierBindings;
	context->identifierBindings = node;
}

void registerCFrontendTypedef(void * scanner, const char * name) {
	_registerBinding(c_yyget_extra(scanner), name, true);
}

void registerCFrontendOrdinary(void * scanner, const char * name) {
	_registerBinding(c_yyget_extra(scanner), name, false);
}

static void _destroyBindings(CIdentifierBinding * bindings) {
	while (bindings != NULL) {
		CIdentifierBinding * next = bindings->next;
		free(bindings->name);
		free(bindings);
		bindings = next;
	}
}

static void _activatePendingParameters(CFrontendContext * context) {
	while (context->pendingParameters != NULL) {
		CIdentifierBinding * parameter = context->pendingParameters;
		context->pendingParameters = parameter->next;
		parameter->scopeDepth = context->scopeDepth;
		parameter->isTypedef = false;
		parameter->next = context->identifierBindings;
		context->identifierBindings = parameter;
	}
}

void prepareCFrontendFunctionParameters(void * scanner, ParameterList * parameters) {
	CFrontendContext * context = c_yyget_extra(scanner);
	if (context == NULL) {
		return;
	}
	_destroyBindings(context->pendingParameters);
	context->pendingParameters = NULL;
	for (ParameterList * parameter = parameters;
		parameter != NULL;
		parameter = parameter->next) {
		if (parameter->parameter == NULL || parameter->parameter->name == NULL) {
			continue;
		}
		CIdentifierBinding * node = calloc(1, sizeof(CIdentifierBinding));
		if (node == NULL) {
			continue;
		}
		node->name = strdup(parameter->parameter->name);
		if (node->name == NULL) {
			free(node);
			continue;
		}
		node->next = context->pendingParameters;
		context->pendingParameters = node;
	}
	if (context->scopeDepth > 0) {
		_activatePendingParameters(context);
	}
}

void pushCFrontendScope(CFrontendContext * context) {
	if (context == NULL) {
		return;
	}
	context->scopeDepth++;
	_activatePendingParameters(context);
}

void popCFrontendScope(CFrontendContext * context) {
	if (context == NULL || context->scopeDepth == 0) {
		return;
	}
	CIdentifierBinding ** link = &context->identifierBindings;
	while (*link != NULL) {
		CIdentifierBinding * node = *link;
		if (node->scopeDepth == context->scopeDepth) {
			*link = node->next;
			node->next = NULL;
			_destroyBindings(node);
			continue;
		}
		link = &node->next;
	}
	context->scopeDepth--;
}

static void _destroyContext(CFrontendContext * context) {
	_destroyBindings(context->identifierBindings);
	_destroyBindings(context->pendingParameters);
	context->identifierBindings = NULL;
	context->pendingParameters = NULL;
}

CompilationStatus executeCFrontend(
	FILE * input,
	const char * sourceName,
	CompilerState * compilerState) {
	CFrontendContext context = {0};
	yyscan_t scanner = NULL;
	if (c_yylex_init_extra(&context, &scanner) != 0) {
		return OUT_OF_MEMORY;
	}
	c_yyset_in(input, scanner);
	int parseStatus = c_yyparse(scanner, compilerState, sourceName);
	c_yylex_destroy(scanner);
	_destroyContext(&context);
	return parseStatus == 0 ? SUCCEEDED : FAILED;
}
