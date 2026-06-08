#ifndef SEMANTIC_SYMBOL_TABLE_HEADER
#define SEMANTIC_SYMBOL_TABLE_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdbool.h>

typedef enum {
	SEMANTIC_SYMBOL_VARIABLE,
	SEMANTIC_SYMBOL_FUNCTION,
	SEMANTIC_SYMBOL_TYPEDEF,
	SEMANTIC_SYMBOL_ENUM_CONSTANT
} SemanticSymbolKind;

typedef enum {
	SEMANTIC_TAG_STRUCT,
	SEMANTIC_TAG_UNION,
	SEMANTIC_TAG_ENUM
} SemanticTagKind;

typedef struct SemanticSymbol SemanticSymbol;
typedef struct SemanticTag SemanticTag;
typedef struct SemanticSymbolTable SemanticSymbolTable;

struct SemanticSymbol {
	char * name;
	SemanticSymbolKind kind;
	Type * type;
	ParameterList * parameters;
	Type * returnType;
	bool hasDefinition;
	SemanticSymbol * next;
};

struct SemanticTag {
	char * name;
	SemanticTagKind kind;
	AggregateDeclaration * declaration;
	SemanticTag * next;
};

SemanticSymbolTable * createSemanticSymbolTable(void);
void destroySemanticSymbolTable(SemanticSymbolTable * table);

void semanticSymbolTablePushScope(SemanticSymbolTable * table);
void semanticSymbolTablePopScope(SemanticSymbolTable * table);

SemanticSymbol * semanticSymbolTableLookupOrdinary(SemanticSymbolTable * table, const char * name);
SemanticSymbol * semanticSymbolTableLookupCurrentOrdinary(SemanticSymbolTable * table, const char * name);
SemanticTag * semanticSymbolTableLookupTag(SemanticSymbolTable * table, const char * name);
SemanticTag * semanticSymbolTableLookupCurrentTag(SemanticSymbolTable * table, const char * name);

bool semanticSymbolTableDeclareOrdinary(
	SemanticSymbolTable * table,
	const char * name,
	SemanticSymbolKind kind,
	Type * type,
	ParameterList * parameters,
	Type * returnType,
	bool hasDefinition);

bool semanticSymbolTableDeclareTag(
	SemanticSymbolTable * table,
	const char * name,
	SemanticTagKind kind,
	AggregateDeclaration * declaration);

#endif
