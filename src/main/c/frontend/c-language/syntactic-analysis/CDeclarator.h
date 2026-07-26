#ifndef C_DECLARATOR_HEADER
#define C_DECLARATOR_HEADER

#include "../../syntactic-analysis/AbstractSyntaxTree.h"

typedef enum {
	C_DECLARATOR_IDENTIFIER,
	C_DECLARATOR_POINTER,
	C_DECLARATOR_ARRAY,
	C_DECLARATOR_FUNCTION
} CDeclaratorKind;

typedef struct CDeclarator CDeclarator;
typedef struct CDeclaratorList CDeclaratorList;
typedef struct CResolvedDeclarator CResolvedDeclarator;
typedef struct CDeclarationSpecifiers CDeclarationSpecifiers;

struct CDeclarator {
	CDeclaratorKind kind;
	char * name;
	CDeclarator * inner;
	Expression * arraySize;
	ParameterList * parameters;
	Expression * initializer;
};

struct CDeclaratorList {
	CDeclarator * declarator;
	CDeclaratorList * next;
};

struct CResolvedDeclarator {
	char * name;
	Type * type;
	ParameterList * parameters;
	Type * returnType;
	Expression * initializer;
	bool isFunction;
};

struct CDeclarationSpecifiers {
	Type * type;
	bool isStatic;
	bool isTypedef;
};

CDeclarator * createCIdentifierDeclarator(char * name);
CDeclarator * createCPointerDeclarator(CDeclarator * inner);
CDeclarator * createCArrayDeclarator(CDeclarator * inner, Expression * size);
CDeclarator * createCFunctionDeclarator(CDeclarator * inner, ParameterList * parameters);
CDeclarator * setCDeclaratorInitializer(CDeclarator * declarator, Expression * initializer);
CDeclaratorList * appendCDeclarator(CDeclaratorList * list, CDeclarator * declarator);
CDeclarationSpecifiers * createCDeclarationSpecifiers(
	Type * type,
	bool isStatic,
	bool isTypedef);
CResolvedDeclarator * resolveCDeclarator(CDeclarator * declarator, Type * baseType);
Type * cloneCType(Type * type);

void destroyCDeclarator(CDeclarator * declarator);
void destroyCDeclaratorList(CDeclaratorList * list);
void destroyCResolvedDeclarator(CResolvedDeclarator * declarator);
void destroyCDeclarationSpecifiers(CDeclarationSpecifiers * specifiers);

#endif
