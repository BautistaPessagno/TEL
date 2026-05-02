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
%token <token> INDENT
%token <token> DEDENT
%token <token> CLOSE_PARENTHESIS
%token <token> ARROW
%token <token> RETURN
%token <token> FUNCTION
%token <token> MAIN
%token <token> TYPE_INT
%token <token> TYPE_CHAR
%token <token> TYPE_FLOAT
%token <token> TYPE_DOUBLE
%token <token> TYPE_VOID
%token <token> TYPE_UINT
%token <token> TYPE_ULI
%token <token> ASSIGN
%token <token> ADD_ASSIGN
%token <token> SUBTRACT_ASSIGN
%token <token> MULTIPLY_ASSIGN
%token <token> DIVIDE_ASSIGN
%token <token> MODULO_ASSIGN
%token <token> BITWISE_AND_ASSIGN
%token <token> BITWISE_OR_ASSIGN
%token <token> BITWISE_XOR_ASSIGN
%token <token> SHIFT_LEFT_ASSIGN
%token <token> SHIFT_RIGHT_ASSIGN
%token <token> LOGICAL_OR
%token <token> LOGICAL_AND
%token <token> BITWISE_OR
%token <token> BITWISE_XOR
%token <token> BITWISE_AND
%token <token> EQUAL
%token <token> NOT_EQUAL
%token <token> LESS_THAN
%token <token> GREATER_THAN
%token <token> LESS_EQUAL
%token <token> GREATER_EQUAL
%token <token> SHIFT_LEFT
%token <token> SHIFT_RIGHT
%token <token> ADD
%token <token> SUBTRACT
%token <token> MULTIPLY
%token <token> DIVIDE
%token <token> MODULO
%token <token> LOGICAL_NOT
%token <token> BITWISE_NOT

%token <token> IGNORED
%token <token> UNKNOWN

%precedence ARGUMENT_BOUNDARY
%nonassoc <token> OPEN_PARENTHESIS
%right ASSIGN ADD_ASSIGN SUBTRACT_ASSIGN MULTIPLY_ASSIGN DIVIDE_ASSIGN MODULO_ASSIGN BITWISE_AND_ASSIGN BITWISE_OR_ASSIGN BITWISE_XOR_ASSIGN SHIFT_LEFT_ASSIGN SHIFT_RIGHT_ASSIGN
%left LOGICAL_OR
%left LOGICAL_AND
%left BITWISE_OR
%left BITWISE_XOR
%left BITWISE_AND
%left EQUAL NOT_EQUAL
%left LESS_THAN GREATER_THAN LESS_EQUAL GREATER_EQUAL
%left SHIFT_LEFT SHIFT_RIGHT
%left ADD SUBTRACT
%left MULTIPLY DIVIDE MODULO
%right LOGICAL_NOT BITWISE_NOT UNARY_PLUS UNARY_MINUS PREFIX_INCREMENT PREFIX_DECREMENT
%left <token> INCREMENT DECREMENT POSTFIX_INCREMENT POSTFIX_DECREMENT

/** Non-terminals. */
%type <type> type
%type <type> optionalReturnType
%type <declaration> declaration
%type <parameter> parameter
%type <parameterList> parameterList
%type <parameterList> optionalParameterList
%type <topLevelItemList> optionalFunctionBody
%type <functionDeclaration> functionDeclaration
%type <functionCall> functionCall
%type <expression> expression
%type <expression> optionalInitializer
%type <expression> optionalReturnExpression
%type <expressionList> argumentList
%type <expressionList> optionalArgumentList
%type <topLevelItem> topLevelItem
%type <topLevelItem> functionBodyItem
%type <topLevelItem> returnStatement
%type <topLevelItem> expressionStatement
%type <topLevelItemList> topLevelItemList
%type <topLevelItemList> functionBodyItemList
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
	| expressionStatement									{ $$ = $1; }
	| SEMICOLON												{ $$ = EmptyStatementTopLevelItemSemanticAction(); }
	;

declaration:
	 IDENTIFIER COLON type optionalInitializer SEMICOLON		{ $$ = VariableDeclarationSemanticAction($1, $3, $4); }
	;

optionalInitializer:
	 %empty													{ $$ = NULL; }
	| ASSIGN expression										{ $$ = $2; }
	;

functionDeclaration:
	 FUNCTION IDENTIFIER optionalParameterList optionalReturnType SEMICOLON optionalFunctionBody	{ $$ = FunctionDeclarationSemanticAction($2, $3, $4, $6); }
	;

optionalFunctionBody:
	 %empty													{ $$ = NULL; }
	| INDENT functionBodyItemList DEDENT					{ $$ = $2; }
	;

functionBodyItemList:
	 functionBodyItem										{ $$ = SingletonTopLevelItemListSemanticAction($1); }
	| functionBodyItemList functionBodyItem					{ $$ = AppendTopLevelItemListSemanticAction($1, $2); }
	;

functionBodyItem:
	 declaration											{ $$ = VariableDeclarationTopLevelItemSemanticAction($1); }
	| functionDeclaration									{ $$ = FunctionDeclarationTopLevelItemSemanticAction($1); }
	| returnStatement										{ $$ = $1; }
	| expressionStatement									{ $$ = $1; }
	| SEMICOLON												{ $$ = EmptyStatementTopLevelItemSemanticAction(); }
	;

returnStatement:
	 RETURN optionalReturnExpression SEMICOLON				{ $$ = ReturnStatementTopLevelItemSemanticAction($2); }
	;

optionalReturnExpression:
	 %empty													{ $$ = NULL; }
	| expression											{ $$ = $1; }
	;

expressionStatement:
	 expression SEMICOLON									{ $$ = ExpressionStatementTopLevelItemSemanticAction($1); }
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
	 expression %prec ARGUMENT_BOUNDARY						{ $$ = SingletonExpressionListSemanticAction($1); }
	| argumentList expression %prec ARGUMENT_BOUNDARY		{ $$ = AppendExpressionListSemanticAction($1, $2); }
	;

expression:
	 IDENTIFIER %prec ARGUMENT_BOUNDARY						{ $$ = IdentifierExpressionSemanticAction($1); }
	| INTEGER_LITERAL										{ $$ = IntegerLiteralExpressionSemanticAction($1); }
	| STRING_LITERAL										{ $$ = StringLiteralExpressionSemanticAction($1); }
	| functionCall											{ $$ = FunctionCallExpressionSemanticAction($1); }
	| OPEN_PARENTHESIS expression CLOSE_PARENTHESIS			{ $$ = $2; }
	| expression ASSIGN expression							{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ASSIGN, $3); }
	| expression ADD_ASSIGN expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ADD_ASSIGN, $3); }
	| expression SUBTRACT_ASSIGN expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SUBTRACT_ASSIGN, $3); }
	| expression MULTIPLY_ASSIGN expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MULTIPLY_ASSIGN, $3); }
	| expression DIVIDE_ASSIGN expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_DIVIDE_ASSIGN, $3); }
	| expression MODULO_ASSIGN expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MODULO_ASSIGN, $3); }
	| expression BITWISE_AND_ASSIGN expression				{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN, $3); }
	| expression BITWISE_OR_ASSIGN expression				{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN, $3); }
	| expression BITWISE_XOR_ASSIGN expression				{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN, $3); }
	| expression SHIFT_LEFT_ASSIGN expression				{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN, $3); }
	| expression SHIFT_RIGHT_ASSIGN expression				{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN, $3); }
	| expression LOGICAL_OR expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LOGICAL_OR, $3); }
	| expression LOGICAL_AND expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LOGICAL_AND, $3); }
	| expression BITWISE_OR expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_OR, $3); }
	| expression BITWISE_XOR expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_XOR, $3); }
	| expression BITWISE_AND expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_AND, $3); }
	| expression EQUAL expression							{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_EQUAL, $3); }
	| expression NOT_EQUAL expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_NOT_EQUAL, $3); }
	| expression LESS_THAN expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LESS_THAN, $3); }
	| expression GREATER_THAN expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_GREATER_THAN, $3); }
	| expression LESS_EQUAL expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LESS_EQUAL, $3); }
	| expression GREATER_EQUAL expression					{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_GREATER_EQUAL, $3); }
	| expression SHIFT_LEFT expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_LEFT, $3); }
	| expression SHIFT_RIGHT expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_RIGHT, $3); }
	| expression ADD expression							{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ADD, $3); }
	| expression SUBTRACT expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SUBTRACT, $3); }
	| expression MULTIPLY expression						{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MULTIPLY, $3); }
	| expression DIVIDE expression							{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_DIVIDE, $3); }
	| expression MODULO expression							{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MODULO, $3); }
	| ADD expression %prec UNARY_PLUS						{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_UNARY_PLUS, $2); }
	| SUBTRACT expression %prec UNARY_MINUS					{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_UNARY_MINUS, $2); }
	| LOGICAL_NOT expression								{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_LOGICAL_NOT, $2); }
	| BITWISE_NOT expression								{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_BITWISE_NOT, $2); }
	| INCREMENT expression %prec PREFIX_INCREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_INCREMENT, $2); }
	| DECREMENT expression %prec PREFIX_DECREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_DECREMENT, $2); }
	| expression INCREMENT %prec POSTFIX_INCREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_INCREMENT, $1); }
	| expression DECREMENT %prec POSTFIX_DECREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_DECREMENT, $1); }
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
