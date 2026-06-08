#include "SemanticSymbolTable.h"
#include <stdlib.h>
#include <string.h>

typedef struct SemanticScope SemanticScope;

struct SemanticScope {
	SemanticSymbol * ordinarySymbols;
	SemanticTag * tags;
	SemanticScope * parent;
};

struct SemanticSymbolTable {
	SemanticScope * currentScope;
};

static SemanticScope * _createScope(SemanticScope * parent);
static void _destroyScope(SemanticScope * scope);
static void _destroyOrdinarySymbols(SemanticSymbol * symbol);
static void _destroyTags(SemanticTag * tag);

static SemanticScope * _createScope(SemanticScope * parent) {
	SemanticScope * scope = calloc(1, sizeof(SemanticScope));
	scope->parent = parent;
	return scope;
}

static void _destroyScope(SemanticScope * scope) {
	if (scope != NULL) {
		_destroyOrdinarySymbols(scope->ordinarySymbols);
		_destroyTags(scope->tags);
		free(scope);
	}
}

static void _destroyOrdinarySymbols(SemanticSymbol * symbol) {
	while (symbol != NULL) {
		SemanticSymbol * next = symbol->next;
		free(symbol->name);
		free(symbol);
		symbol = next;
	}
}

static void _destroyTags(SemanticTag * tag) {
	while (tag != NULL) {
		SemanticTag * next = tag->next;
		free(tag->name);
		free(tag);
		tag = next;
	}
}

SemanticSymbolTable * createSemanticSymbolTable(void) {
	SemanticSymbolTable * table = calloc(1, sizeof(SemanticSymbolTable));
	return table;
}

void destroySemanticSymbolTable(SemanticSymbolTable * table) {
	if (table != NULL) {
		while (table->currentScope != NULL) {
			semanticSymbolTablePopScope(table);
		}
		free(table);
	}
}

void semanticSymbolTablePushScope(SemanticSymbolTable * table) {
	table->currentScope = _createScope(table->currentScope);
}

void semanticSymbolTablePopScope(SemanticSymbolTable * table) {
	if (table != NULL && table->currentScope != NULL) {
		SemanticScope * parent = table->currentScope->parent;
		_destroyScope(table->currentScope);
		table->currentScope = parent;
	}
}

SemanticSymbol * semanticSymbolTableLookupOrdinary(SemanticSymbolTable * table, const char * name) {
	if (table == NULL || name == NULL) {
		return NULL;
	}
	for (SemanticScope * scope = table->currentScope; scope != NULL; scope = scope->parent) {
		for (SemanticSymbol * symbol = scope->ordinarySymbols; symbol != NULL; symbol = symbol->next) {
			if (strcmp(symbol->name, name) == 0) {
				return symbol;
			}
		}
	}
	return NULL;
}

SemanticSymbol * semanticSymbolTableLookupCurrentOrdinary(SemanticSymbolTable * table, const char * name) {
	if (table == NULL || table->currentScope == NULL || name == NULL) {
		return NULL;
	}
	for (SemanticSymbol * symbol = table->currentScope->ordinarySymbols; symbol != NULL; symbol = symbol->next) {
		if (strcmp(symbol->name, name) == 0) {
			return symbol;
		}
	}
	return NULL;
}

SemanticTag * semanticSymbolTableLookupTag(SemanticSymbolTable * table, const char * name) {
	if (table == NULL || name == NULL) {
		return NULL;
	}
	for (SemanticScope * scope = table->currentScope; scope != NULL; scope = scope->parent) {
		for (SemanticTag * tag = scope->tags; tag != NULL; tag = tag->next) {
			if (strcmp(tag->name, name) == 0) {
				return tag;
			}
		}
	}
	return NULL;
}

SemanticTag * semanticSymbolTableLookupCurrentTag(SemanticSymbolTable * table, const char * name) {
	if (table == NULL || table->currentScope == NULL || name == NULL) {
		return NULL;
	}
	for (SemanticTag * tag = table->currentScope->tags; tag != NULL; tag = tag->next) {
		if (strcmp(tag->name, name) == 0) {
			return tag;
		}
	}
	return NULL;
}

bool semanticSymbolTableDeclareOrdinary(
	SemanticSymbolTable * table,
	const char * name,
	SemanticSymbolKind kind,
	Type * type,
	ParameterList * parameters,
	Type * returnType,
	bool hasDefinition) {
	if (table == NULL || table->currentScope == NULL || name == NULL) {
		return false;
	}
	if (semanticSymbolTableLookupCurrentOrdinary(table, name) != NULL) {
		return false;
	}
	SemanticSymbol * symbol = calloc(1, sizeof(SemanticSymbol));
	symbol->name = strdup(name);
	symbol->kind = kind;
	symbol->type = type;
	symbol->parameters = parameters;
	symbol->returnType = returnType;
	symbol->hasDefinition = hasDefinition;
	symbol->next = table->currentScope->ordinarySymbols;
	table->currentScope->ordinarySymbols = symbol;
	return true;
}

bool semanticSymbolTableDeclareTag(
	SemanticSymbolTable * table,
	const char * name,
	SemanticTagKind kind,
	AggregateDeclaration * declaration) {
	if (table == NULL || table->currentScope == NULL || name == NULL) {
		return false;
	}
	if (semanticSymbolTableLookupCurrentTag(table, name) != NULL) {
		return false;
	}
	SemanticTag * tag = calloc(1, sizeof(SemanticTag));
	tag->name = strdup(name);
	tag->kind = kind;
	tag->declaration = declaration;
	tag->next = table->currentScope->tags;
	table->currentScope->tags = tag;
	return true;
}
