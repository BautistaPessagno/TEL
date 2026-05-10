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
	Parameter * parameter;
	ParameterList * parameterList;
	FunctionDeclaration * functionDeclaration;
	MainDeclaration * mainDeclaration;
	AggregateDeclaration * aggregateDeclaration;
	EnumMember * enumMember;
	EnumMemberList * enumMemberList;
	EnumDeclaration * enumDeclaration;
	TypedefDeclaration * typedefDeclaration;
	PreprocessorDirective * preprocessorDirective;
	FunctionCall * functionCall;
	Expression * expression;
	ExpressionList * expressionList;
	ProgramItem * programItem;
	ProgramItemList * programItemList;
	IfBranch * ifBranch;
	ForStatement * forStatement;
	WhileStatement * whileStatement;
	DoWhileStatement * doWhileStatement;
	SwitchCase * switchCase;
	SwitchStatement * switchStatement;
	Statement * statement;
	StatementList * statementList;
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
%destructor { destroyParameter($$); } <parameter>
%destructor { destroyParameterList($$); } <parameterList>
%destructor { destroyFunctionDeclaration($$); } <functionDeclaration>
%destructor { destroyMainDeclaration($$); } <mainDeclaration>
%destructor { destroyAggregateDeclaration($$); } <aggregateDeclaration>
%destructor { destroyEnumMember($$); } <enumMember>
%destructor { destroyEnumMemberList($$); } <enumMemberList>
%destructor { destroyEnumDeclaration($$); } <enumDeclaration>
%destructor { destroyTypedefDeclaration($$); } <typedefDeclaration>
%destructor { destroyPreprocessorDirective($$); } <preprocessorDirective>
%destructor { destroyFunctionCall($$); } <functionCall>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyExpressionList($$); } <expressionList>
%destructor { destroyProgramItem($$); } <programItem>
%destructor { destroyProgramItemList($$); } <programItemList>
%destructor { destroyIfBranch($$); } <ifBranch>
%destructor { destroyForStatement($$); } <forStatement>
%destructor { destroyWhileStatement($$); } <whileStatement>
%destructor { destroyDoWhileStatement($$); } <doWhileStatement>
%destructor { destroySwitchCase($$); } <switchCase>
%destructor { destroySwitchStatement($$); } <switchStatement>
%destructor { destroyStatement($$); } <statement>
%destructor { destroyStatementList($$); } <statementList>

/** Terminals. */
%token <string> IDENTIFIER
%token <string> TYPEDEF_NAME
%token <string> INTEGER_LITERAL
%token <string> FLOAT_LITERAL
%token <string> CHAR_LITERAL
%token <string> STRING_LITERAL
%token <string> INCLUDE_DIRECTIVE
%token <string> DEFINE_DIRECTIVE
%token <token> COLON
%token <token> COMMA
%token <token> ASSIGN
%token <token> SEMICOLON
%token <token> LINEBREAK
%token <token> INDENT
%token <token> DEDENT
%token <token> OPEN_PARENTHESIS
%token <token> CLOSE_PARENTHESIS
%token <token> OPEN_BRACKET
%token <token> CLOSE_BRACKET
%token <token> OPEN_BRACE
%token <token> CLOSE_BRACE
%token <token> ARROW
%token <token> PTR_ARROW
%token <token> DOT
%token <token> RETURN
%token <token> IF
%token <token> ELIF
%token <token> ELSE
%token <token> FORD
%token <token> FOR
%token <token> WHILE
%token <token> DO_WHILE
%token <token> SWITCH
%token <token> DEFAULT
%token <token> BREAK
%token <token> CONTINUE
%token <token> FUNCTION
%token <token> FUNCTION_POINTER
%token <token> MAIN
%token <token> STRUCT
%token <token> ENUM
%token <token> UNION
%token <token> TYPEDEF
%token <token> TYPE_INT
%token <token> TYPE_CHAR
%token <token> TYPE_FLOAT
%token <token> TYPE_DOUBLE
%token <token> TYPE_VOID
%token <token> TYPE_UINT
%token <token> TYPE_ULI
%token <token> TYPE_LONG
%token <token> NULL_LITERAL
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
%token <token> INCREMENT
%token <token> DECREMENT

%token <token> IGNORED
%token <token> UNKNOWN

%precedence ARGUMENT_BOUNDARY
%nonassoc OPEN_PARENTHESIS
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
%right LOGICAL_NOT BITWISE_NOT UNARY_PLUS UNARY_MINUS UNARY_DEREFERENCE UNARY_ADDRESS_OF PREFIX_INCREMENT PREFIX_DECREMENT
/* OPEN_BRACKET sits at postfix precedence so `type OPEN_BRACKET ...` and
 * `expression OPEN_BRACKET expression CLOSE_BRACKET %prec ARRAY_INDEX` resolve
 * by default-shift; removing it reintroduces shift/reduce conflicts on `[`. */
%left INCREMENT DECREMENT POSTFIX_INCREMENT POSTFIX_DECREMENT ARRAY_INDEX MEMBER_ACCESS POINTER_MEMBER_ACCESS OPEN_BRACKET DOT PTR_ARROW

/** Non-terminals. */
%type <string> identifier
%type <token> terminator
%type <token> arrow
%type <type> type
%type <type> optionalReturnType
%type <expression> optionalInitializer
%type <declaration> declaration
%type <declarationList> variableDeclarationList
%type <parameter> parameter
%type <parameterList> parameterList
%type <parameterList> optionalParameterList
%type <parameterList> bareTypeList
%type <declaration> functionPointerDeclaration
%type <statementList> optionalFunctionBody
%type <statementList> functionBody
%type <functionDeclaration> functionDeclaration
%type <mainDeclaration> mainDeclaration
%type <aggregateDeclaration> aggregateDeclaration
%type <enumMember> enumMember
%type <enumMemberList> enumMemberList
%type <enumDeclaration> enumDeclaration
%type <typedefDeclaration> typedefDeclaration
%type <preprocessorDirective> preprocessorDirective
%type <string> optionalEnumMemberValue
%type <functionCall> functionCall
%type <expression> expression
%type <expression> fordBound
%type <expression> optionalReturnExpression
%type <expressionList> callArgumentList
%type <expressionList> optionalCallArgumentList
%type <expressionList> argumentList
%type <expressionList> optionalArgumentList
%type <programItem> programItem
%type <statement> statement
%type <statement> returnStatement
%type <statement> expressionStatement
%type <statement> ifStatement
%type <statement> fordStatement
%type <statement> forStatement
%type <statement> whileStatement
%type <statement> doWhileStatement
%type <statement> switchStatement
%type <statement> switchInlineStatement
%type <statement> breakStatement
%type <statement> continueStatement
%type <ifBranch> ifBranch
%type <ifBranch> elifBranch
%type <ifBranch> optionalElifBranchList
%type <ifBranch> elifBranchList
%type <statementList> optionalElseBody
%type <forStatement> fordLoop
%type <forStatement> forLoop
%type <whileStatement> whileLoop
%type <doWhileStatement> doWhileLoop
%type <switchCase> switchCase
%type <switchCase> switchCaseList
%type <statementList> switchCaseBody
%type <statementList> switchInlineStatementList
%type <programItemList> optionalProgramItemList
%type <programItemList> programItemList
%type <statementList> statementList
%type <program> program

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
	 optionalProgramItemList								{ $$ = ProgramSemanticAction($1); }
	| optionalProgramItemList mainDeclaration optionalProgramItemList	{ $$ = ProgramSemanticAction(ConcatenateProgramItemListSemanticAction(AppendProgramItemListSemanticAction($1, MainDeclarationProgramItemSemanticAction($2)), $3)); }
	;

identifier:
	 IDENTIFIER												{ $$ = $1; }
	| TYPEDEF_NAME											{ $$ = $1; }
	;

optionalProgramItemList:
	 %empty													{ $$ = NULL; }
	| programItemList										{ $$ = $1; }
	;

programItemList:
	 programItem											{ $$ = SingletonProgramItemListSemanticAction($1); }
	| programItemList programItem							{ $$ = AppendProgramItemListSemanticAction($1, $2); }
	;

programItem:
	 declaration											{ $$ = VariableDeclarationProgramItemSemanticAction($1); }
	| functionDeclaration									{ $$ = FunctionDeclarationProgramItemSemanticAction($1); }
	| functionPointerDeclaration							{ $$ = VariableDeclarationProgramItemSemanticAction($1); }
	| aggregateDeclaration									{ $$ = AggregateDeclarationProgramItemSemanticAction($1); }
	| enumDeclaration										{ $$ = EnumDeclarationProgramItemSemanticAction($1); }
	| typedefDeclaration									{ $$ = TypedefDeclarationProgramItemSemanticAction($1); }
	| preprocessorDirective									{ $$ = PreprocessorDirectiveProgramItemSemanticAction($1); }
	| terminator											{ $$ = EmptyProgramItemSemanticAction(); }
	;

declaration:
	 identifier COLON type optionalInitializer terminator	{ $$ = VariableDeclarationSemanticAction($1, $3, $4); }
	;

optionalInitializer:
	 %empty													{ $$ = NULL; }
	| ASSIGN expression										{ $$ = $2; }
	;

variableDeclarationList:
	 declaration											{ $$ = SingletonVariableDeclarationListSemanticAction($1); }
	| variableDeclarationList declaration					{ $$ = AppendVariableDeclarationListSemanticAction($1, $2); }
	| variableDeclarationList terminator						{ $$ = $1; }
	;

functionDeclaration:
	 FUNCTION identifier optionalParameterList optionalReturnType terminator optionalFunctionBody	{ $$ = FunctionDeclarationSemanticAction($2, $3, $4, $6); }
	;

optionalFunctionBody:
	 %empty													{ $$ = NULL; }
	| functionBody											{ $$ = $1; }
	;

functionBody:
	 INDENT statementList DEDENT							{ $$ = FunctionBodyStatementListSemanticAction($2); }
	;

mainDeclaration:
	 MAIN LINEBREAK functionBody							{ $$ = MainDeclarationSemanticAction($3); }
	;

statementList:
	 statement												{ $$ = SingletonStatementListSemanticAction($1); }
	| statementList statement								{ $$ = AppendStatementListSemanticAction($1, $2); }
	;

statement:
	 declaration											{ $$ = VariableDeclarationStatementSemanticAction($1); }
	| functionPointerDeclaration							{ $$ = VariableDeclarationStatementSemanticAction($1); }
	| returnStatement										{ $$ = $1; }
	| expressionStatement									{ $$ = $1; }
	| ifStatement											{ $$ = $1; }
	| fordStatement											{ $$ = $1; }
	| forStatement											{ $$ = $1; }
	| whileStatement										{ $$ = $1; }
	| doWhileStatement										{ $$ = $1; }
	| switchStatement										{ $$ = $1; }
	| breakStatement										{ $$ = $1; }
	| continueStatement										{ $$ = $1; }
	| terminator											{ $$ = EmptyStatementSemanticAction(); }
	;

returnStatement:
	 RETURN optionalReturnExpression terminator				{ $$ = ReturnStatementSemanticAction($2); }
	;

optionalReturnExpression:
	 %empty													{ $$ = NULL; }
	| expression											{ $$ = $1; }
	;

expressionStatement:
	 expression terminator									{ $$ = ExpressionStatementSemanticAction($1); }
	;

ifStatement:
	 ifBranch optionalElifBranchList optionalElseBody		{ $$ = IfStatementSemanticActionWrapper(IfStatementSemanticAction(AppendIfBranchSemanticAction($1, $2), $3)); }
	;

ifBranch:
	 IF expression terminator INDENT statementList DEDENT	{ $$ = IfBranchSemanticAction($2, $5); }
	;

optionalElifBranchList:
	 %empty													{ $$ = NULL; }
	| elifBranchList										{ $$ = $1; }
	;

elifBranchList:
	 elifBranch												{ $$ = $1; }
	| elifBranchList elifBranch								{ $$ = AppendIfBranchSemanticAction($1, $2); }
	;

elifBranch:
	 ELIF expression terminator INDENT statementList DEDENT	{ $$ = IfBranchSemanticAction($2, $5); }
	;

optionalElseBody:
	 %empty													{ $$ = NULL; }
	| ELSE terminator INDENT statementList DEDENT			{ $$ = $4; }
	;

fordStatement:
	 fordLoop												{ $$ = ForStatementSemanticActionWrapper($1); }
	;

fordLoop:
	 FORD identifier fordBound fordBound terminator INDENT statementList DEDENT	{ $$ = ForStatementFromFordSemanticAction($2, $3, $4, $7); }
	;

fordBound:
	 identifier												{ $$ = IdentifierExpressionSemanticAction($1); }
	| INTEGER_LITERAL										{ $$ = IntegerLiteralExpressionSemanticAction($1); }
	| OPEN_PARENTHESIS expression CLOSE_PARENTHESIS			{ $$ = $2; }
	;

forStatement:
	 forLoop												{ $$ = ForStatementSemanticActionWrapper($1); }
	;

forLoop:
	 FOR expression COMMA expression COMMA expression terminator INDENT statementList DEDENT	{ $$ = ForStatementSemanticAction($2, $4, $6, $9); }
	;

whileStatement:
	 whileLoop												{ $$ = WhileStatementSemanticActionWrapper($1); }
	;

whileLoop:
	 WHILE expression terminator INDENT statementList DEDENT	{ $$ = WhileStatementSemanticAction($2, $5); }
	;

doWhileStatement:
	 doWhileLoop											{ $$ = DoWhileStatementSemanticActionWrapper($1); }
	;

// The first expression after the dedent is intentionally the dw condition.
doWhileLoop:
	 DO_WHILE terminator INDENT statementList DEDENT expression terminator	{ $$ = DoWhileStatementSemanticAction($4, $6); }
	;

switchStatement:
	 SWITCH expression terminator INDENT switchCaseList DEDENT	{ $$ = SwitchStatementSemanticActionWrapper(SwitchStatementSemanticAction($2, $5)); }
	;

switchCaseList:
	 switchCase												{ $$ = $1; }
	| switchCaseList switchCase								{ $$ = AppendSwitchCaseSemanticAction($1, $2); }
	| switchCaseList LINEBREAK								{ $$ = $1; }
	;

switchCase:
	 expression COLON switchCaseBody						{ $$ = SwitchCaseSemanticAction($1, $3); }
	| expression ARROW switchCaseBody						{ $$ = SwitchCaseSemanticAction($1, AppendStatementListSemanticAction($3, BreakStatementSemanticAction())); }
	| DEFAULT COLON switchCaseBody							{ $$ = SwitchCaseSemanticAction(NULL, $3); }
	| DEFAULT ARROW switchCaseBody							{ $$ = SwitchCaseSemanticAction(NULL, AppendStatementListSemanticAction($3, BreakStatementSemanticAction())); }
	| DEFAULT BREAK terminator								{ $$ = SwitchCaseSemanticAction(NULL, SingletonStatementListSemanticAction(BreakStatementSemanticAction())); }
	;

switchCaseBody:
	 terminator INDENT statementList DEDENT					{ $$ = $3; }
	| switchInlineStatementList LINEBREAK					{ $$ = $1; }
	;

switchInlineStatementList:
	 switchInlineStatement									{ $$ = SingletonStatementListSemanticAction($1); }
	| switchInlineStatementList SEMICOLON switchInlineStatement	{ $$ = AppendStatementListSemanticAction($1, $3); }
	;

switchInlineStatement:
	 identifier COLON type optionalInitializer				{ $$ = VariableDeclarationStatementSemanticAction(VariableDeclarationSemanticAction($1, $3, $4)); }
	| RETURN optionalReturnExpression						{ $$ = ReturnStatementSemanticAction($2); }
	| expression											{ $$ = ExpressionStatementSemanticAction($1); }
	| BREAK													{ $$ = BreakStatementSemanticAction(); }
	| CONTINUE												{ $$ = ContinueStatementSemanticAction(); }
	;

breakStatement:
	 BREAK terminator										{ $$ = BreakStatementSemanticAction(); }
	;

continueStatement:
	 CONTINUE terminator									{ $$ = ContinueStatementSemanticAction(); }
	;

terminator:
	 SEMICOLON												{ $$ = $1; }
	| LINEBREAK												{ $$ = $1; }
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
	 identifier COLON type									{ $$ = ParameterSemanticAction($1, $3); }
	;

bareTypeList:
	 type													{ $$ = SingletonBareParameterListSemanticAction($1); }
	| bareTypeList type										{ $$ = AppendBareParameterListSemanticAction($1, $2); }
	;

functionPointerDeclaration:
	 FUNCTION_POINTER identifier bareTypeList arrow type terminator
		{ $$ = VariableDeclarationSemanticAction($2, FunctionPointerTypeSemanticAction($3, $5), NULL); }
	| FUNCTION_POINTER identifier arrow type terminator
		{ $$ = VariableDeclarationSemanticAction($2, FunctionPointerTypeSemanticAction(NULL, $4), NULL); }
	| FUNCTION_POINTER identifier bareTypeList terminator
		{ $$ = VariableDeclarationSemanticAction($2, FunctionPointerTypeSemanticAction($3, TypeSemanticAction(TYPE_VOID_KIND)), NULL); }
	| FUNCTION_POINTER identifier terminator
		{ $$ = VariableDeclarationSemanticAction($2, FunctionPointerTypeSemanticAction(NULL, TypeSemanticAction(TYPE_VOID_KIND)), NULL); }
	;

optionalReturnType:
	 %empty													{ $$ = TypeSemanticAction(TYPE_VOID_KIND); }
	| arrow type											{ $$ = $2; }
	;

arrow:
	 ARROW													{ $$ = $1; }
	| PTR_ARROW												{ $$ = $1; }
	;

aggregateDeclaration:
	 STRUCT identifier terminator INDENT variableDeclarationList DEDENT		{ $$ = AggregateDeclarationSemanticAction(AGGREGATE_STRUCT_KIND, $2, $5); }
	| UNION identifier terminator INDENT variableDeclarationList DEDENT		{ $$ = AggregateDeclarationSemanticAction(AGGREGATE_UNION_KIND, $2, $5); }
	;

enumDeclaration:
	 ENUM identifier terminator INDENT enumMemberList DEDENT	{ $$ = EnumDeclarationSemanticAction($2, $5); }
	;

enumMemberList:
	 enumMember												{ $$ = SingletonEnumMemberListSemanticAction($1); }
	| enumMemberList enumMember								{ $$ = AppendEnumMemberListSemanticAction($1, $2); }
	| enumMemberList terminator								{ $$ = $1; }
	;

enumMember:
	 identifier optionalEnumMemberValue terminator			{ $$ = EnumMemberSemanticAction($1, $2); }
	;

optionalEnumMemberValue:
	 %empty													{ $$ = NULL; }
	| ASSIGN INTEGER_LITERAL								{ $$ = $2; }
	;

typedefDeclaration:
	 TYPEDEF identifier COLON type terminator				{ $$ = TypedefDeclarationSemanticAction($2, $4); }
	;

preprocessorDirective:
	 INCLUDE_DIRECTIVE terminator							{ $$ = PreprocessorDirectiveSemanticAction(PREPROCESSOR_INCLUDE_DIRECTIVE, $1); }
	| DEFINE_DIRECTIVE terminator							{ $$ = PreprocessorDirectiveSemanticAction(PREPROCESSOR_DEFINE_DIRECTIVE, $1); }
	;

functionCall:
	 identifier OPEN_PARENTHESIS optionalCallArgumentList CLOSE_PARENTHESIS	{ $$ = FunctionCallSemanticAction($1, $3); }
	;

optionalCallArgumentList:
	 %empty													{ $$ = NULL; }
	| callArgumentList										{ $$ = $1; }
	;

callArgumentList:
	 expression												{ $$ = SingletonExpressionListSemanticAction($1); }
	| callArgumentList COMMA expression						{ $$ = AppendExpressionListSemanticAction($1, $3); }
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
	 identifier %prec ARGUMENT_BOUNDARY						{ $$ = IdentifierExpressionSemanticAction($1); }
	| INTEGER_LITERAL										{ $$ = IntegerLiteralExpressionSemanticAction($1); }
	| FLOAT_LITERAL											{ $$ = FloatLiteralExpressionSemanticAction($1); }
	| CHAR_LITERAL											{ $$ = CharLiteralExpressionSemanticAction($1); }
	| STRING_LITERAL										{ $$ = StringLiteralExpressionSemanticAction($1); }
	| NULL_LITERAL											{ $$ = NullLiteralExpressionSemanticAction(); }
	| OPEN_BRACE optionalArgumentList CLOSE_BRACE			{ $$ = ArrayLiteralExpressionSemanticAction($2); }
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
	| MULTIPLY expression %prec UNARY_DEREFERENCE			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_DEREFERENCE, $2); }
	| BITWISE_AND expression %prec UNARY_ADDRESS_OF			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_ADDRESS_OF, $2); }
	| LOGICAL_NOT expression								{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_LOGICAL_NOT, $2); }
	| BITWISE_NOT expression								{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_BITWISE_NOT, $2); }
	| INCREMENT expression %prec PREFIX_INCREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_INCREMENT, $2); }
	| DECREMENT expression %prec PREFIX_DECREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_DECREMENT, $2); }
	| expression INCREMENT %prec POSTFIX_INCREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_INCREMENT, $1); }
	| expression DECREMENT %prec POSTFIX_DECREMENT			{ $$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_DECREMENT, $1); }
	| expression OPEN_BRACKET expression CLOSE_BRACKET %prec ARRAY_INDEX	{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ARRAY_INDEX, $3); }
	| expression DOT identifier %prec MEMBER_ACCESS			{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MEMBER_ACCESS, IdentifierExpressionSemanticAction($3)); }
	| expression PTR_ARROW identifier %prec POINTER_MEMBER_ACCESS	{ $$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS, IdentifierExpressionSemanticAction($3)); }
	;

type:
	 TYPE_INT												{ $$ = TypeSemanticAction(TYPE_INT_KIND); }
	| TYPE_CHAR												{ $$ = TypeSemanticAction(TYPE_CHAR_KIND); }
	| TYPE_FLOAT											{ $$ = TypeSemanticAction(TYPE_FLOAT_KIND); }
	| TYPE_DOUBLE											{ $$ = TypeSemanticAction(TYPE_DOUBLE_KIND); }
	| TYPE_VOID												{ $$ = TypeSemanticAction(TYPE_VOID_KIND); }
	| TYPE_UINT												{ $$ = TypeSemanticAction(TYPE_UINT_KIND); }
	| TYPE_ULI												{ $$ = TypeSemanticAction(TYPE_ULI_KIND); }
	| TYPE_LONG												{ $$ = TypeSemanticAction(TYPE_LONG_KIND); }
	| TYPEDEF_NAME											{ $$ = NamedTypeSemanticAction(TYPE_NAMED_KIND, $1); }
	| STRUCT identifier										{ $$ = NamedTypeSemanticAction(TYPE_STRUCT_KIND, $2); }
	| ENUM identifier										{ $$ = NamedTypeSemanticAction(TYPE_ENUM_KIND, $2); }
	| UNION identifier										{ $$ = NamedTypeSemanticAction(TYPE_UNION_KIND, $2); }
	/* TODO: compound forms like `int*[3]`, `int[3][4]`, `int[]*` parse via these
	 * recursive rules but their AST shape is not yet a defined contract. Pin
	 * desired semantics and add accept/reject fixtures before relying on them. */
	| type MULTIPLY %prec UNARY_DEREFERENCE					{ $$ = PointerTypeSemanticAction($1); }
	| type OPEN_BRACKET expression CLOSE_BRACKET			{ $$ = ArrayTypeSemanticAction($1, $3); }
	| type OPEN_BRACKET CLOSE_BRACKET						{ $$ = ArrayTypeSemanticAction($1, NULL); }
	| OPEN_PARENTHESIS FUNCTION_POINTER bareTypeList arrow type CLOSE_PARENTHESIS
		{ $$ = FunctionPointerTypeSemanticAction($3, $5); }
	| OPEN_PARENTHESIS FUNCTION_POINTER arrow type CLOSE_PARENTHESIS
		{ $$ = FunctionPointerTypeSemanticAction(NULL, $4); }
	| OPEN_PARENTHESIS FUNCTION_POINTER bareTypeList CLOSE_PARENTHESIS
		{ $$ = FunctionPointerTypeSemanticAction($3, TypeSemanticAction(TYPE_VOID_KIND)); }
	| OPEN_PARENTHESIS FUNCTION_POINTER CLOSE_PARENTHESIS
		{ $$ = FunctionPointerTypeSemanticAction(NULL, TypeSemanticAction(TYPE_VOID_KIND)); }
	;

%%
