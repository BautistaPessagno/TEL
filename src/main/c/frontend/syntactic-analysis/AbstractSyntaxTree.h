#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * This type definitions allows self-referencing types (e.g., an expression
 * that is made of another expressions, such as talking about you in 3rd
 * person, but without the madness).
 */

typedef enum TypeKind TypeKind;

typedef struct Type Type;
typedef struct VariableDeclaration VariableDeclaration;
typedef struct VariableDeclarationList VariableDeclarationList;
typedef struct Program Program;

/**
 * Node types for the Abstract Syntax Tree (AST).
 */

enum TypeKind {
	TYPE_INT_KIND,
	TYPE_CHAR_KIND,
	TYPE_FLOAT_KIND,
	TYPE_DOUBLE_KIND,
	TYPE_VOID_KIND,
	TYPE_UINT_KIND,
	TYPE_ULI_KIND
};

struct Type {
	TypeKind kind;
};

struct VariableDeclaration {
	char * name;
	Type * type;
};

struct VariableDeclarationList {
	VariableDeclaration * declaration;
	VariableDeclarationList * next;
};

struct Program {
	VariableDeclarationList * declarations;
};

/**
 * Node recursive super-duper-trambolik-destructors.
 */

void destroyType(Type * type);
void destroyVariableDeclaration(VariableDeclaration * variableDeclaration);
void destroyVariableDeclarationList(VariableDeclarationList * variableDeclarationList);
void destroyProgram(Program * program);

#endif
