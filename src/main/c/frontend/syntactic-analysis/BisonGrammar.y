%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"
#include <stdlib.h>

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
	/** Terminals. */

	char * string;
	TokenLabel token;

	/** Non-terminals. */

	Type * type;
	VariableDeclaration * declaration;
	VariableDeclarationList * declarationList;
	Program * program;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { free($$); } <string>
%destructor { destroyType($$); } <type>
%destructor { destroyVariableDeclaration($$); } <declaration>
%destructor { destroyVariableDeclarationList($$); } <declarationList>

/** Terminals. */
%token <string> IDENTIFIER
%token <token> COLON
%token <token> SEMICOLON
%token <token> TYPE_INT
%token <token> TYPE_CHAR
%token <token> TYPE_FLOAT
%token <token> TYPE_DOUBLE
%token <token> TYPE_VOID
%token <token> TYPE_UINT
%token <token> TYPE_ULI

%token <token> IGNORED
%token <token> UNKNOWN

/** Non-terminals. */
%type <type> type
%type <declaration> declaration
%type <declarationList> declarationList
%type <program> program

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
	 declarationList										{ $$ = ProgramSemanticAction($1); }
	;

declarationList:
	 declaration											{ $$ = SingletonDeclarationListSemanticAction($1); }
	| declarationList declaration							{ $$ = AppendDeclarationListSemanticAction($1, $2); }
	;

declaration:
	 IDENTIFIER COLON type SEMICOLON						{ $$ = VariableDeclarationSemanticAction($1, $3); }
	;

type:
	 TYPE_INT												{ $$ = TypeSemanticAction(TYPE_INT_KIND); }
	| TYPE_CHAR												{ $$ = TypeSemanticAction(TYPE_CHAR_KIND); }
	| TYPE_FLOAT											{ $$ = TypeSemanticAction(TYPE_FLOAT_KIND); }
	| TYPE_DOUBLE											{ $$ = TypeSemanticAction(TYPE_DOUBLE_KIND); }
	| TYPE_VOID												{ $$ = TypeSemanticAction(TYPE_VOID_KIND); }
	| TYPE_UINT												{ $$ = TypeSemanticAction(TYPE_UINT_KIND); }
	| TYPE_ULI												{ $$ = TypeSemanticAction(TYPE_ULI_KIND); }
	;

%%
