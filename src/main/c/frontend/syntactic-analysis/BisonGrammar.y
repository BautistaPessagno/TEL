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
	Parameter * parameter;
	ParameterList * parameterList;
	FunctionDeclaration * functionDeclaration;
	FunctionCall * functionCall;
	Expression * expression;
	ExpressionList * expressionList;
	TopLevelItem * topLevelItem;
	TopLevelItemList * topLevelItemList;
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
%destructor { destroyParameter($$); } <parameter>
%destructor { destroyParameterList($$); } <parameterList>
%destructor { destroyFunctionDeclaration($$); } <functionDeclaration>
%destructor { destroyFunctionCall($$); } <functionCall>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyExpressionList($$); } <expressionList>
%destructor { destroyTopLevelItem($$); } <topLevelItem>
%destructor { destroyTopLevelItemList($$); } <topLevelItemList>

/** Terminals. */
%token <string> IDENTIFIER
%token <string> INTEGER_LITERAL
%token <string> STRING_LITERAL
%token <token> COLON
%token <token> SEMICOLON
%token <token> OPEN_PARENTHESIS
%token <token> CLOSE_PARENTHESIS
%token <token> ARROW
%token <token> FUNCTION
%token <token> MAIN
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
%type <type> optionalReturnType
%type <declaration> declaration
%type <parameter> parameter
%type <parameterList> parameterList
%type <parameterList> optionalParameterList
%type <functionDeclaration> functionDeclaration
%type <functionCall> functionCall
%type <expression> expression
%type <expressionList> argumentList
%type <expressionList> optionalArgumentList
%type <topLevelItem> topLevelItem
%type <topLevelItemList> topLevelItemList
%type <program> program

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
	 topLevelItemList										{ $$ = ProgramSemanticAction($1); }
	;

topLevelItemList:
	 topLevelItem											{ $$ = SingletonTopLevelItemListSemanticAction($1); }
	| topLevelItemList topLevelItem							{ $$ = AppendTopLevelItemListSemanticAction($1, $2); }
	;

topLevelItem:
	 declaration											{ $$ = VariableDeclarationTopLevelItemSemanticAction($1); }
	| functionDeclaration									{ $$ = FunctionDeclarationTopLevelItemSemanticAction($1); }
	| functionCall SEMICOLON								{ $$ = FunctionCallTopLevelItemSemanticAction($1); }
	;

declaration:
	 IDENTIFIER COLON type SEMICOLON						{ $$ = VariableDeclarationSemanticAction($1, $3); }
	;

functionDeclaration:
	 FUNCTION IDENTIFIER optionalParameterList optionalReturnType SEMICOLON	{ $$ = FunctionDeclarationSemanticAction($2, $3, $4); }
	;

optionalParameterList:
	 %empty													{ $$ = NULL; }
	| parameterList											{ $$ = $1; }
	;

parameterList:
	 parameter												{ $$ = SingletonParameterListSemanticAction($1); }
	| parameterList parameter								{ $$ = AppendParameterListSemanticAction($1, $2); }
	;

parameter:
	 IDENTIFIER COLON type									{ $$ = ParameterSemanticAction($1, $3); }
	;

optionalReturnType:
	 %empty													{ $$ = TypeSemanticAction(TYPE_VOID_KIND); }
	| ARROW type											{ $$ = $2; }
	;

functionCall:
	 IDENTIFIER OPEN_PARENTHESIS optionalArgumentList CLOSE_PARENTHESIS	{ $$ = FunctionCallSemanticAction($1, $3); }
	;

optionalArgumentList:
	 %empty													{ $$ = NULL; }
	| argumentList											{ $$ = $1; }
	;

argumentList:
	 expression												{ $$ = SingletonExpressionListSemanticAction($1); }
	| argumentList expression								{ $$ = AppendExpressionListSemanticAction($1, $2); }
	;

expression:
	 IDENTIFIER												{ $$ = IdentifierExpressionSemanticAction($1); }
	| INTEGER_LITERAL										{ $$ = IntegerLiteralExpressionSemanticAction($1); }
	| STRING_LITERAL										{ $$ = StringLiteralExpressionSemanticAction($1); }
	| functionCall											{ $$ = FunctionCallExpressionSemanticAction($1); }
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
