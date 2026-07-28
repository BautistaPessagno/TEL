%require "3.8.2"
%define api.prefix {c_yy}
%define api.pure full
%define parse.error detailed
%locations

%parse-param { yyscan_t scanner }
%parse-param { CompilerState * compilerState }
%parse-param { const char * sourceName }
%lex-param { yyscan_t scanner }

%code requires {
	#include "../../../support/type/CompilerState.h"
	#include "../../syntactic-analysis/AbstractSyntaxTree.h"
	#include "CDeclarator.h"
	typedef void * yyscan_t;
}

%code {
	#include "../../syntactic-analysis/AstBuilder.h"
	#include "../CFrontend.h"
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>

	int c_yylex(C_YYSTYPE * value, C_YYLTYPE * location, yyscan_t scanner);

	static bool parameterNameExists(ParameterList * parameters, const char * name) {
		for (ParameterList * node = parameters; node != NULL; node = node->next) {
			if (node->parameter != NULL
				&& node->parameter->name != NULL
				&& strcmp(node->parameter->name, name) == 0) {
				return true;
			}
		}
		return false;
	}

	static bool nameUnnamedParameters(ParameterList * parameters) {
		unsigned int candidate = 0;
		for (ParameterList * node = parameters; node != NULL; node = node->next) {
			if (node->parameter == NULL || node->parameter->name != NULL) {
				continue;
			}
			char name[32];
			do {
				snprintf(name, sizeof(name), "_p%u", candidate++);
			}
			while (parameterNameExists(parameters, name));
			node->parameter->name = strdup(name);
			if (node->parameter->name == NULL) {
				return false;
			}
		}
		return true;
	}

	static void prepareFunctionParameters(void * scanner, CDeclarator * declarator) {
		for (CDeclarator * node = declarator; node != NULL; node = node->inner) {
			if (node->kind == C_DECLARATOR_FUNCTION
				&& node->inner != NULL
				&& node->inner->kind == C_DECLARATOR_IDENTIFIER) {
				prepareCFrontendFunctionParameters(scanner, node->parameters);
				return;
			}
		}
		prepareCFrontendFunctionParameters(scanner, NULL);
	}

	static ProgramItemList * buildProgramItems(
		CDeclarationSpecifiers * specifiers,
		CDeclaratorList * declarators,
		yyscan_t scanner) {
		ProgramItemList * items = NULL;
		CDeclaratorList * node = declarators;
		while (node != NULL) {
			CDeclaratorList * next = node->next;
			CResolvedDeclarator * resolved = resolveCDeclarator(
				node->declarator,
				cloneCType(specifiers->type));
			node->declarator = NULL;
			free(node);
			if (resolved == NULL
				|| (specifiers->isTypedef
					&& (resolved->isFunction
						|| resolved->initializer != NULL
						|| specifiers->isStatic))
				|| (resolved->isFunction
					&& (resolved->initializer != NULL
						|| strcmp(resolved->name, "main") == 0))) {
				destroyCResolvedDeclarator(resolved);
				destroyProgramItemList(items);
				destroyCDeclaratorList(next);
				destroyCDeclarationSpecifiers(specifiers);
				return NULL;
			}
			ProgramItem * item;
			if (specifiers->isTypedef) {
				registerCFrontendTypedef(scanner, resolved->name);
				TypedefDeclaration * declaration = TypedefDeclarationSemanticAction(
					resolved->name,
					resolved->type);
				resolved->name = NULL;
				resolved->type = NULL;
				item = TypedefDeclarationProgramItemSemanticAction(declaration);
			}
			else if (resolved->isFunction) {
				if (!nameUnnamedParameters(resolved->parameters)) {
					destroyCResolvedDeclarator(resolved);
					destroyProgramItemList(items);
					destroyCDeclaratorList(next);
					destroyCDeclarationSpecifiers(specifiers);
					return NULL;
				}
				FunctionDeclaration * declaration = FunctionDeclarationSemanticAction(
					resolved->name,
					resolved->parameters,
					resolved->returnType,
					NULL,
					specifiers->isStatic);
				resolved->name = NULL;
				resolved->parameters = NULL;
				resolved->returnType = NULL;
				item = FunctionDeclarationProgramItemSemanticAction(declaration);
			}
			else {
				VariableDeclaration * declaration = VariableDeclarationSemanticAction(
					resolved->name,
					resolved->type,
					resolved->initializer,
					specifiers->isStatic);
				resolved->name = NULL;
				resolved->type = NULL;
				resolved->initializer = NULL;
				item = VariableDeclarationProgramItemSemanticAction(declaration);
			}
			destroyCResolvedDeclarator(resolved);
			items = AppendProgramItemListSemanticAction(items, item);
			node = next;
		}
		destroyCDeclarationSpecifiers(specifiers);
		return items;
	}

	static VariableDeclarationList * buildAggregateFields(
		CDeclarationSpecifiers * specifiers,
		CDeclaratorList * declarators) {
		if (specifiers->isStatic || specifiers->isTypedef) {
			destroyCDeclarationSpecifiers(specifiers);
			destroyCDeclaratorList(declarators);
			return NULL;
		}
		VariableDeclarationList * fields = NULL;
		CDeclaratorList * node = declarators;
		while (node != NULL) {
			CDeclaratorList * next = node->next;
			CResolvedDeclarator * resolved = resolveCDeclarator(
				node->declarator,
				cloneCType(specifiers->type));
			node->declarator = NULL;
			free(node);
			if (resolved == NULL
				|| resolved->isFunction
				|| resolved->initializer != NULL) {
				destroyCResolvedDeclarator(resolved);
				destroyVariableDeclarationList(fields);
				destroyCDeclaratorList(next);
				destroyCDeclarationSpecifiers(specifiers);
				return NULL;
			}
			VariableDeclaration * field = VariableDeclarationSemanticAction(
				resolved->name,
				resolved->type,
				NULL,
				false);
			resolved->name = NULL;
			resolved->type = NULL;
			destroyCResolvedDeclarator(resolved);
			fields = AppendVariableDeclarationListSemanticAction(fields, field);
			node = next;
		}
		destroyCDeclarationSpecifiers(specifiers);
		return fields;
	}

	static StatementList * buildDeclarationStatements(
		CDeclarationSpecifiers * specifiers,
		CDeclaratorList * declarators) {
		if (specifiers->isTypedef) {
			destroyCDeclarationSpecifiers(specifiers);
			destroyCDeclaratorList(declarators);
			return NULL;
		}
		StatementList * statements = NULL;
		CDeclaratorList * node = declarators;
		while (node != NULL) {
			CDeclaratorList * next = node->next;
			CResolvedDeclarator * resolved = resolveCDeclarator(
				node->declarator,
				cloneCType(specifiers->type));
			node->declarator = NULL;
			free(node);
			if (resolved == NULL || resolved->isFunction) {
				destroyCResolvedDeclarator(resolved);
				destroyStatementList(statements);
				destroyCDeclaratorList(next);
				destroyCDeclarationSpecifiers(specifiers);
				return NULL;
			}
			VariableDeclaration * declaration = VariableDeclarationSemanticAction(
				resolved->name,
				resolved->type,
				resolved->initializer,
				specifiers->isStatic);
			resolved->name = NULL;
			resolved->type = NULL;
			resolved->initializer = NULL;
			destroyCResolvedDeclarator(resolved);
			statements = AppendStatementListSemanticAction(
				statements,
				VariableDeclarationStatementSemanticAction(declaration));
			node = next;
		}
		destroyCDeclarationSpecifiers(specifiers);
		return statements;
	}

	static ProgramItem * buildFunctionDefinition(
		CDeclarationSpecifiers * specifiers,
		CDeclarator * declarator,
		StatementList * body) {
		CResolvedDeclarator * resolved = resolveCDeclarator(
			declarator,
			cloneCType(specifiers->type));
		if (resolved == NULL || !resolved->isFunction || specifiers->isTypedef) {
			destroyCResolvedDeclarator(resolved);
			destroyCDeclarationSpecifiers(specifiers);
			destroyStatementList(body);
			return NULL;
		}
		ProgramItem * item;
		if (strcmp(resolved->name, "main") == 0) {
			bool validParameters = resolved->parameters == NULL;
			if (!validParameters
				&& resolved->parameters->next != NULL
				&& resolved->parameters->next->next == NULL) {
				Type * first = resolved->parameters->parameter->type;
				Type * second = resolved->parameters->next->parameter->type;
				const char * firstName = resolved->parameters->parameter->name;
				const char * secondName = resolved->parameters->next->parameter->name;
				bool firstIsInt = first != NULL
					&& first->kind == TYPE_INT_KIND
					&& firstName != NULL
					&& strcmp(firstName, "argc") == 0;
				bool secondIsCharArgumentVector = false;
				if (second != NULL && second->kind == TYPE_ARRAY_KIND) {
					second = second->pointee;
					secondIsCharArgumentVector = second != NULL
						&& second->kind == TYPE_POINTER_KIND
						&& second->pointee != NULL
						&& second->pointee->kind == TYPE_CHAR_KIND;
				}
				else if (second != NULL && second->kind == TYPE_POINTER_KIND) {
					second = second->pointee;
					if (second != NULL && second->kind == TYPE_POINTER_KIND) {
						second = second->pointee;
						secondIsCharArgumentVector = second != NULL
							&& second->kind == TYPE_CHAR_KIND;
					}
				}
				validParameters = firstIsInt
					&& secondIsCharArgumentVector
					&& secondName != NULL
					&& strcmp(secondName, "argv") == 0;
			}
			if (specifiers->isStatic
				|| resolved->returnType == NULL
				|| resolved->returnType->kind != TYPE_INT_KIND
				|| !validParameters) {
				destroyCResolvedDeclarator(resolved);
				destroyCDeclarationSpecifiers(specifiers);
				destroyStatementList(body);
				return NULL;
			}
			item = MainDeclarationProgramItemSemanticAction(MainDeclarationSemanticAction(body));
			body = NULL;
			}
			else {
				if (!nameUnnamedParameters(resolved->parameters)) {
					destroyCResolvedDeclarator(resolved);
					destroyCDeclarationSpecifiers(specifiers);
					destroyStatementList(body);
					return NULL;
				}
				FunctionDeclaration * declaration = FunctionDeclarationSemanticAction(
				resolved->name,
				resolved->parameters,
				resolved->returnType,
				body,
				specifiers->isStatic);
			resolved->name = NULL;
			resolved->parameters = NULL;
			resolved->returnType = NULL;
			body = NULL;
			item = FunctionDeclarationProgramItemSemanticAction(declaration);
		}
		destroyCResolvedDeclarator(resolved);
		destroyCDeclarationSpecifiers(specifiers);
		destroyStatementList(body);
		return item;
	}

	static Statement * buildIfStatement(
		Expression * condition,
		StatementList * body,
		StatementList * elseBody) {
		IfBranch * branches = IfBranchSemanticAction(condition, body);
		if (elseBody != NULL
			&& elseBody->next == NULL
			&& elseBody->statement != NULL
			&& elseBody->statement->kind == STATEMENT_IF) {
			IfStatement * nested = elseBody->statement->ifStatement;
			branches = AppendIfBranchSemanticAction(branches, nested->branches);
			nested->branches = NULL;
			StatementList * nestedElse = nested->elseBody;
			nested->elseBody = NULL;
			destroyStatementList(elseBody);
			elseBody = nestedElse;
		}
		return IfStatementSemanticActionWrapper(
			IfStatementSemanticAction(branches, elseBody));
	}

	static Statement * buildCountingFor(
		char * iterator,
		Expression * start,
		Expression * condition,
		Expression * update,
		StatementList * body) {
		bool canonicalCondition = condition != NULL
			&& condition->kind == EXPRESSION_BINARY_OPERATION
			&& condition->operator == EXPRESSION_OPERATOR_LESS_THAN
			&& condition->left != NULL
			&& condition->left->kind == EXPRESSION_IDENTIFIER
			&& strcmp(condition->left->value, iterator) == 0;
		bool canonicalUpdate = update != NULL
			&& update->kind == EXPRESSION_UNARY_OPERATION
			&& update->operator == EXPRESSION_OPERATOR_POSTFIX_INCREMENT
			&& update->operand != NULL
			&& update->operand->kind == EXPRESSION_IDENTIFIER
			&& strcmp(update->operand->value, iterator) == 0;
		if (!canonicalCondition || !canonicalUpdate) {
			free(iterator);
			destroyExpression(start);
			destroyExpression(condition);
			destroyExpression(update);
			destroyStatementList(body);
			return NULL;
		}
		Expression * end = condition->right;
		condition->right = NULL;
		destroyExpression(condition);
		destroyExpression(update);
		return ForStatementSemanticActionWrapper(
			ForStatementFromFordSemanticAction(iterator, start, end, body));
	}

	void c_yyerror(
		C_YYLTYPE * location,
		yyscan_t scanner,
		CompilerState * compilerState,
		const char * sourceName,
		const char * message) {
		(void) scanner;
		(void) compilerState;
		fprintf(stderr, "%s:%d: %s\n", sourceName, location->first_line, message);
	}
}

%union {
	char * string;
	unsigned int count;
	Type * type;
	CDeclarationSpecifiers * declarationSpecifiers;
	CDeclarator * declarator;
	CDeclaratorList * declaratorList;
	Parameter * parameter;
	ParameterList * parameterList;
	VariableDeclarationList * declarationList;
	EnumMember * enumMember;
	EnumMemberList * enumMemberList;
	FunctionDeclaration * functionDeclaration;
	Expression * expression;
	ExpressionList * expressionList;
	Statement * statement;
	StatementList * statementList;
	SwitchCase * switchCase;
	ProgramItem * programItem;
	ProgramItemList * programItemList;
}

%token <string> C_IDENTIFIER C_TYPEDEF_NAME C_INTEGER_LITERAL C_FLOAT_LITERAL C_CHAR_LITERAL C_STRING_LITERAL
%token <string> C_INCLUDE_DIRECTIVE C_DEFINE_DIRECTIVE C_INLINE_C_BLOCK
%token C_INT C_CHAR C_FLOAT C_DOUBLE C_VOID C_UNSIGNED C_LONG C_SHORT C_CONST C_STATIC C_TYPEDEF
%token C_STRUCT C_UNION C_ENUM
%token C_RETURN C_NULL
%token C_IF C_ELSE C_FOR C_WHILE C_DO C_SWITCH C_CASE C_DEFAULT C_BREAK C_CONTINUE
%token C_ASSIGN C_ADD_ASSIGN C_SUBTRACT_ASSIGN C_MULTIPLY_ASSIGN C_DIVIDE_ASSIGN C_MODULO_ASSIGN
%token C_BITWISE_AND_ASSIGN C_BITWISE_OR_ASSIGN C_BITWISE_XOR_ASSIGN
%token C_SHIFT_LEFT_ASSIGN C_SHIFT_RIGHT_ASSIGN
%token C_LOGICAL_OR C_LOGICAL_AND C_BITWISE_OR C_BITWISE_XOR C_BITWISE_AND
%token C_EQUAL C_NOT_EQUAL C_LESS_THAN C_GREATER_THAN C_LESS_EQUAL C_GREATER_EQUAL
%token C_SHIFT_LEFT C_SHIFT_RIGHT C_ADD C_SUBTRACT C_MULTIPLY C_DIVIDE C_MODULO
%token C_LOGICAL_NOT C_BITWISE_NOT C_INCREMENT C_DECREMENT C_ARROW C_DOT
%token C_OPEN_PARENTHESIS C_CLOSE_PARENTHESIS C_OPEN_BRACKET C_CLOSE_BRACKET
%token C_OPEN_BRACE C_CLOSE_BRACE C_COMMA C_COLON C_SEMICOLON

%type <programItemList> translationUnit externalDeclarationList externalDeclaration
%type <programItem> functionDefinition aggregateDeclaration enumDeclaration preprocessorDirective
%type <type> typeSpecifier
%type <string> cIdentifier
%type <declarationSpecifiers> declarationSpecifiers
%type <declarator> declarator directDeclarator abstractDeclarator initDeclarator localInitDeclarator
%type <declaratorList> initDeclaratorList localInitDeclaratorList
%type <parameter> parameterDeclaration
%type <parameterList> parameterList
%type <declarationList> aggregateFieldDeclarationList aggregateFieldDeclaration
%type <enumMember> enumMember
%type <enumMemberList> enumMemberList
%type <statementList> compoundStatement optionalStatementList statementList
%type <statementList> statement statementBody optionalElseBody
%type <statement> returnStatement expressionStatement ifStatement forStatement
%type <statement> whileStatement doWhileStatement switchStatement breakStatement continueStatement
%type <switchCase> switchCase switchCaseList
%type <expression> optionalExpression expression initializer
%type <expressionList> optionalArgumentList argumentList optionalInitializerList initializerList

%destructor { free($$); } <string>
%destructor { destroyType($$); } <type>
%destructor { destroyCDeclarationSpecifiers($$); } <declarationSpecifiers>
%destructor { destroyCDeclarator($$); } <declarator>
%destructor { destroyCDeclaratorList($$); } <declaratorList>
%destructor { destroyParameter($$); } <parameter>
%destructor { destroyParameterList($$); } <parameterList>
%destructor { destroyVariableDeclarationList($$); } <declarationList>
%destructor { destroyEnumMember($$); } <enumMember>
%destructor { destroyEnumMemberList($$); } <enumMemberList>
%destructor { destroyExpression($$); } <expression>
%destructor { destroyExpressionList($$); } <expressionList>
%destructor { destroyStatement($$); } <statement>
%destructor { destroyStatementList($$); } <statementList>
%destructor { destroySwitchCase($$); } <switchCase>
%destructor { destroyProgramItem($$); } <programItem>
%destructor { destroyProgramItemList($$); } <programItemList>

%right C_ASSIGN C_ADD_ASSIGN C_SUBTRACT_ASSIGN C_MULTIPLY_ASSIGN C_DIVIDE_ASSIGN
%right C_MODULO_ASSIGN C_BITWISE_AND_ASSIGN C_BITWISE_OR_ASSIGN C_BITWISE_XOR_ASSIGN
%right C_SHIFT_LEFT_ASSIGN C_SHIFT_RIGHT_ASSIGN
%left C_LOGICAL_OR
%left C_LOGICAL_AND
%left C_BITWISE_OR
%left C_BITWISE_XOR
%left C_BITWISE_AND
%left C_EQUAL C_NOT_EQUAL
%left C_LESS_THAN C_GREATER_THAN C_LESS_EQUAL C_GREATER_EQUAL
%left C_SHIFT_LEFT C_SHIFT_RIGHT
%left C_ADD C_SUBTRACT
%left C_MULTIPLY C_DIVIDE C_MODULO
%right C_LOGICAL_NOT C_BITWISE_NOT C_UNARY_PLUS C_UNARY_MINUS
%right C_UNARY_DEREFERENCE C_UNARY_ADDRESS_OF C_PREFIX_INCREMENT C_PREFIX_DECREMENT
%left C_INCREMENT C_DECREMENT C_POSTFIX_INCREMENT C_POSTFIX_DECREMENT
%left C_ARRAY_INDEX C_MEMBER_ACCESS C_OPEN_BRACKET C_DOT C_ARROW
%nonassoc C_WITHOUT_ELSE
%nonassoc C_ELSE

%start translationUnit

%%

translationUnit:
	externalDeclarationList {
		Program * program = calloc(1, sizeof(Program));
		program->items = $1;
		compilerState->abstractSyntaxtTree = program;
		$$ = NULL;
	}
	;

externalDeclarationList:
	externalDeclaration {
		$$ = $1;
	}
	| externalDeclarationList externalDeclaration {
		$$ = ConcatenateProgramItemListSemanticAction($1, $2);
	}
	;

externalDeclaration:
	declarationSpecifiers initDeclaratorList C_SEMICOLON {
		$$ = buildProgramItems($1, $2, scanner);
		if ($$ == NULL) {
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported declaration");
			YYERROR;
		}
	}
	| functionDefinition {
		$$ = SingletonProgramItemListSemanticAction($1);
	}
	| aggregateDeclaration {
		$$ = SingletonProgramItemListSemanticAction($1);
	}
	| enumDeclaration {
		$$ = SingletonProgramItemListSemanticAction($1);
	}
	| preprocessorDirective {
		$$ = SingletonProgramItemListSemanticAction($1);
	}
	| C_INLINE_C_BLOCK {
		$$ = SingletonProgramItemListSemanticAction(
			InlineCProgramItemSemanticAction($1));
	}
	;

functionDefinition:
	declarationSpecifiers declarator {
		prepareFunctionParameters(scanner, $2);
	} compoundStatement {
		$$ = buildFunctionDefinition($1, $2, $4);
		if ($$ == NULL) {
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported function definition");
			YYERROR;
		}
	}
	;

declarationSpecifiers:
	typeSpecifier {
		$$ = createCDeclarationSpecifiers($1, false, false);
	}
	| C_STATIC typeSpecifier {
		$$ = createCDeclarationSpecifiers($2, true, false);
	}
	| C_TYPEDEF typeSpecifier {
		$$ = createCDeclarationSpecifiers($2, false, true);
	}
	;

typeSpecifier:
	C_INT { $$ = TypeSemanticAction(TYPE_INT_KIND); }
	| C_CHAR { $$ = TypeSemanticAction(TYPE_CHAR_KIND); }
	| C_FLOAT { $$ = TypeSemanticAction(TYPE_FLOAT_KIND); }
	| C_DOUBLE { $$ = TypeSemanticAction(TYPE_DOUBLE_KIND); }
	| C_VOID { $$ = TypeSemanticAction(TYPE_VOID_KIND); }
	| C_LONG optionalInt { $$ = TypeSemanticAction(TYPE_LONG_KIND); }
	| C_SHORT optionalInt { $$ = TypeSemanticAction(TYPE_SHORT_KIND); }
	| C_UNSIGNED optionalInt { $$ = TypeSemanticAction(TYPE_UINT_KIND); }
	| C_UNSIGNED C_LONG optionalInt { $$ = TypeSemanticAction(TYPE_ULI_KIND); }
	| C_CONST typeSpecifier { $$ = ConstQualifiedTypeSemanticAction($2); }
	| C_TYPEDEF_NAME { $$ = NamedTypeSemanticAction(TYPE_NAMED_KIND, $1); }
	| C_STRUCT cIdentifier { $$ = NamedTypeSemanticAction(TYPE_STRUCT_KIND, $2); }
	| C_UNION cIdentifier { $$ = NamedTypeSemanticAction(TYPE_UNION_KIND, $2); }
	| C_ENUM cIdentifier { $$ = NamedTypeSemanticAction(TYPE_ENUM_KIND, $2); }
	;

cIdentifier:
	C_IDENTIFIER { $$ = $1; }
	| C_TYPEDEF_NAME { $$ = $1; }
	;

optionalInt:
	%empty
	| C_INT
	;

aggregateDeclaration:
	C_STRUCT cIdentifier C_OPEN_BRACE aggregateFieldDeclarationList C_CLOSE_BRACE C_SEMICOLON {
		$$ = AggregateDeclarationProgramItemSemanticAction(
			AggregateDeclarationSemanticAction(AGGREGATE_STRUCT_KIND, $2, $4));
	}
	| C_UNION cIdentifier C_OPEN_BRACE aggregateFieldDeclarationList C_CLOSE_BRACE C_SEMICOLON {
		$$ = AggregateDeclarationProgramItemSemanticAction(
			AggregateDeclarationSemanticAction(AGGREGATE_UNION_KIND, $2, $4));
	}
	;

aggregateFieldDeclarationList:
	aggregateFieldDeclaration { $$ = $1; }
	| aggregateFieldDeclarationList aggregateFieldDeclaration {
		VariableDeclarationList * tail = $1;
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = $2;
		$$ = $1;
	}
	;

aggregateFieldDeclaration:
	declarationSpecifiers initDeclaratorList C_SEMICOLON {
		$$ = buildAggregateFields($1, $2);
		if ($$ == NULL) {
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported aggregate field");
			YYERROR;
		}
	}
	;

enumDeclaration:
	C_ENUM cIdentifier C_OPEN_BRACE enumMemberList C_CLOSE_BRACE C_SEMICOLON {
		$$ = EnumDeclarationProgramItemSemanticAction(
			EnumDeclarationSemanticAction($2, $4));
	}
	| C_ENUM cIdentifier C_OPEN_BRACE enumMemberList C_COMMA C_CLOSE_BRACE C_SEMICOLON {
		$$ = EnumDeclarationProgramItemSemanticAction(
			EnumDeclarationSemanticAction($2, $4));
	}
	;

enumMemberList:
	enumMember { $$ = SingletonEnumMemberListSemanticAction($1); }
	| enumMemberList C_COMMA enumMember {
		$$ = AppendEnumMemberListSemanticAction($1, $3);
	}
	;

enumMember:
	cIdentifier { $$ = EnumMemberSemanticAction($1, NULL); }
	| cIdentifier C_ASSIGN C_INTEGER_LITERAL {
		$$ = EnumMemberSemanticAction($1, $3);
	}
	;

preprocessorDirective:
	C_INCLUDE_DIRECTIVE {
		$$ = PreprocessorDirectiveProgramItemSemanticAction(
			PreprocessorDirectiveSemanticAction(PREPROCESSOR_INCLUDE_DIRECTIVE, $1));
	}
	| C_DEFINE_DIRECTIVE {
		$$ = PreprocessorDirectiveProgramItemSemanticAction(
			PreprocessorDirectiveSemanticAction(PREPROCESSOR_DEFINE_DIRECTIVE, $1));
	}
	;

initDeclaratorList:
	initDeclarator {
		$$ = appendCDeclarator(NULL, $1);
	}
	| initDeclaratorList C_COMMA initDeclarator {
		$$ = appendCDeclarator($1, $3);
	}
	;

localInitDeclaratorList:
	localInitDeclarator {
		$$ = appendCDeclarator(NULL, $1);
	}
	| localInitDeclaratorList C_COMMA localInitDeclarator {
		$$ = appendCDeclarator($1, $3);
	}
	;

localInitDeclarator:
	declarator {
		registerCFrontendOrdinary(scanner, cDeclaratorName($1));
		$$ = $1;
	}
	| declarator {
		registerCFrontendOrdinary(scanner, cDeclaratorName($1));
	} C_ASSIGN initializer {
		$$ = setCDeclaratorInitializer($1, $4);
	}
	;

initDeclarator:
	declarator {
		$$ = $1;
	}
	| declarator C_ASSIGN initializer {
		$$ = setCDeclaratorInitializer($1, $3);
	}
	;

initializer:
	expression { $$ = $1; }
	| C_OPEN_BRACE optionalInitializerList C_CLOSE_BRACE {
		$$ = ArrayLiteralExpressionSemanticAction($2);
	}
	;

optionalInitializerList:
	%empty { $$ = NULL; }
	| initializerList { $$ = $1; }
	;

initializerList:
	initializer { $$ = SingletonExpressionListSemanticAction($1); }
	| initializerList C_COMMA initializer {
		$$ = AppendExpressionListSemanticAction($1, $3);
	}
	| initializerList C_COMMA {
		$$ = $1;
	}
	;

declarator:
	directDeclarator {
		$$ = $1;
	}
	| C_MULTIPLY declarator {
		$$ = createCPointerDeclarator($2);
	}
	;

directDeclarator:
	cIdentifier {
		$$ = createCIdentifierDeclarator($1);
	}
	| C_OPEN_PARENTHESIS declarator C_CLOSE_PARENTHESIS {
		$$ = $2;
	}
	| directDeclarator C_OPEN_BRACKET optionalExpression C_CLOSE_BRACKET {
		$$ = createCArrayDeclarator($1, $3);
	}
	| directDeclarator C_OPEN_PARENTHESIS C_CLOSE_PARENTHESIS {
		destroyCDeclarator($1);
		$$ = NULL;
		c_yyerror(
			&@$,
			scanner,
			compilerState,
			sourceName,
			"function declarators require an explicit parameter list");
		YYERROR;
	}
	| directDeclarator C_OPEN_PARENTHESIS parameterList C_CLOSE_PARENTHESIS {
		$$ = createCFunctionDeclarator($1, $3);
	}
	;

parameterList:
	parameterDeclaration { $$ = SingletonParameterListSemanticAction($1); }
	| parameterList C_COMMA parameterDeclaration {
		$$ = AppendParameterListSemanticAction($1, $3);
	}
	;

parameterDeclaration:
	typeSpecifier declarator {
		CResolvedDeclarator * resolved = resolveCDeclarator($2, $1);
		if (resolved == NULL || resolved->isFunction || resolved->initializer != NULL) {
			destroyCResolvedDeclarator(resolved);
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported parameter declarator");
			YYERROR;
		}
		$$ = ParameterSemanticAction(resolved->name, resolved->type);
		resolved->name = NULL;
		resolved->type = NULL;
		destroyCResolvedDeclarator(resolved);
	}
	| typeSpecifier {
		$$ = ParameterSemanticAction(NULL, $1);
	}
	| typeSpecifier abstractDeclarator {
		CResolvedDeclarator * resolved = resolveCDeclarator($2, $1);
		if (resolved == NULL || resolved->isFunction || resolved->initializer != NULL) {
			destroyCResolvedDeclarator(resolved);
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported abstract declarator");
			YYERROR;
		}
		$$ = ParameterSemanticAction(NULL, resolved->type);
		resolved->type = NULL;
		destroyCResolvedDeclarator(resolved);
	}
	;

abstractDeclarator:
	C_MULTIPLY {
		$$ = createCPointerDeclarator(createCIdentifierDeclarator(NULL));
	}
	| C_MULTIPLY abstractDeclarator {
		$$ = createCPointerDeclarator($2);
	}
	| C_OPEN_BRACKET optionalExpression C_CLOSE_BRACKET {
		$$ = createCArrayDeclarator(createCIdentifierDeclarator(NULL), $2);
	}
	| abstractDeclarator C_OPEN_BRACKET optionalExpression C_CLOSE_BRACKET {
		$$ = createCArrayDeclarator($1, $3);
	}
	| C_OPEN_PARENTHESIS abstractDeclarator C_CLOSE_PARENTHESIS {
		$$ = $2;
	}
	;

compoundStatement:
	C_OPEN_BRACE optionalStatementList C_CLOSE_BRACE {
		$$ = $2 != NULL
			? $2
			: SingletonStatementListSemanticAction(
				InlineCStatementSemanticAction(strdup(";\n")));
	}
	;

optionalStatementList:
	%empty { $$ = NULL; }
	| statementList { $$ = $1; }
	;

statementList:
	statement { $$ = $1; }
	| statementList statement {
		$$ = ConcatenateStatementListSemanticAction($1, $2);
	}
	;

statement:
	returnStatement { $$ = SingletonStatementListSemanticAction($1); }
	| expressionStatement { $$ = SingletonStatementListSemanticAction($1); }
	| ifStatement { $$ = SingletonStatementListSemanticAction($1); }
	| forStatement { $$ = SingletonStatementListSemanticAction($1); }
	| whileStatement { $$ = SingletonStatementListSemanticAction($1); }
	| doWhileStatement { $$ = SingletonStatementListSemanticAction($1); }
	| switchStatement { $$ = SingletonStatementListSemanticAction($1); }
	| breakStatement { $$ = SingletonStatementListSemanticAction($1); }
	| continueStatement { $$ = SingletonStatementListSemanticAction($1); }
	| C_INLINE_C_BLOCK {
		$$ = SingletonStatementListSemanticAction(
			InlineCStatementSemanticAction($1));
	}
	| declarationSpecifiers localInitDeclaratorList C_SEMICOLON {
		$$ = buildDeclarationStatements($1, $2);
		if ($$ == NULL) {
			c_yyerror(&@$, scanner, compilerState, sourceName, "unsupported local declaration");
			YYERROR;
		}
	}
	;

statementBody:
	compoundStatement { $$ = $1; }
	| statement { $$ = $1; }
	;

returnStatement:
	C_RETURN optionalExpression C_SEMICOLON {
		$$ = ReturnStatementSemanticAction($2);
	}
	;

expressionStatement:
	expression C_SEMICOLON {
		$$ = ExpressionStatementSemanticAction($1);
	}
	;

ifStatement:
	C_IF C_OPEN_PARENTHESIS expression C_CLOSE_PARENTHESIS statementBody optionalElseBody {
		$$ = buildIfStatement($3, $5, $6);
	}
	;

optionalElseBody:
	%empty %prec C_WITHOUT_ELSE { $$ = NULL; }
	| C_ELSE statementBody { $$ = $2; }
	;

forStatement:
	C_FOR C_OPEN_PARENTHESIS expression C_SEMICOLON expression C_SEMICOLON expression C_CLOSE_PARENTHESIS statementBody {
		$$ = ForStatementSemanticActionWrapper(
			ForStatementSemanticAction($3, $5, $7, $9));
	}
	| C_FOR C_OPEN_PARENTHESIS C_INT cIdentifier C_ASSIGN expression C_SEMICOLON expression C_SEMICOLON expression C_CLOSE_PARENTHESIS statementBody {
		$$ = buildCountingFor($4, $6, $8, $10, $12);
		if ($$ == NULL) {
			c_yyerror(&@$, scanner, compilerState, sourceName, "non-representable declared for initializer");
			YYERROR;
		}
	}
	;

whileStatement:
	C_WHILE C_OPEN_PARENTHESIS expression C_CLOSE_PARENTHESIS statementBody {
		$$ = WhileStatementSemanticActionWrapper(
			WhileStatementSemanticAction($3, $5));
	}
	;

doWhileStatement:
	C_DO statementBody C_WHILE C_OPEN_PARENTHESIS expression C_CLOSE_PARENTHESIS C_SEMICOLON {
		$$ = DoWhileStatementSemanticActionWrapper(
			DoWhileStatementSemanticAction($2, $5));
	}
	;

switchStatement:
	C_SWITCH C_OPEN_PARENTHESIS expression C_CLOSE_PARENTHESIS
	C_OPEN_BRACE switchCaseList C_CLOSE_BRACE {
		$$ = SwitchStatementSemanticActionWrapper(
			SwitchStatementSemanticAction($3, $6));
	}
	;

switchCaseList:
	switchCase { $$ = $1; }
	| switchCaseList switchCase {
		$$ = AppendSwitchCaseSemanticAction($1, $2);
	}
	;

switchCase:
	C_CASE expression C_COLON compoundStatement {
		$$ = SwitchCaseSemanticAction($2, $4);
	}
	| C_CASE expression C_COLON statementList {
		$$ = SwitchCaseSemanticAction($2, $4);
	}
	| C_DEFAULT C_COLON compoundStatement {
		$$ = SwitchCaseSemanticAction(NULL, $3);
	}
	| C_DEFAULT C_COLON statementList {
		$$ = SwitchCaseSemanticAction(NULL, $3);
	}
	;

breakStatement:
	C_BREAK C_SEMICOLON {
		$$ = BreakStatementSemanticAction();
	}
	;

continueStatement:
	C_CONTINUE C_SEMICOLON {
		$$ = ContinueStatementSemanticAction();
	}
	;

optionalExpression:
	%empty { $$ = NULL; }
	| expression { $$ = $1; }
	;

optionalArgumentList:
	%empty { $$ = NULL; }
	| argumentList { $$ = $1; }
	;

argumentList:
	expression { $$ = SingletonExpressionListSemanticAction($1); }
	| argumentList C_COMMA expression {
		$$ = AppendExpressionListSemanticAction($1, $3);
	}
	;

expression:
	C_IDENTIFIER {
		$$ = IdentifierExpressionSemanticAction($1);
	}
	| C_INTEGER_LITERAL {
		$$ = IntegerLiteralExpressionSemanticAction($1);
	}
	| C_FLOAT_LITERAL {
		$$ = FloatLiteralExpressionSemanticAction($1);
	}
	| C_CHAR_LITERAL {
		$$ = CharLiteralExpressionSemanticAction($1);
	}
	| C_STRING_LITERAL {
		$$ = StringLiteralExpressionSemanticAction($1);
	}
	| C_NULL {
		$$ = NullLiteralExpressionSemanticAction();
	}
	| C_IDENTIFIER C_OPEN_PARENTHESIS optionalArgumentList C_CLOSE_PARENTHESIS {
		$$ = FunctionCallExpressionSemanticAction(FunctionCallSemanticAction($1, $3));
	}
	| C_OPEN_PARENTHESIS expression C_CLOSE_PARENTHESIS {
		$$ = $2;
	}
	| expression C_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ASSIGN, $3);
	}
	| expression C_ADD_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ADD_ASSIGN, $3);
	}
	| expression C_SUBTRACT_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SUBTRACT_ASSIGN, $3);
	}
	| expression C_MULTIPLY_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MULTIPLY_ASSIGN, $3);
	}
	| expression C_DIVIDE_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_DIVIDE_ASSIGN, $3);
	}
	| expression C_MODULO_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MODULO_ASSIGN, $3);
	}
	| expression C_BITWISE_AND_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN, $3);
	}
	| expression C_BITWISE_OR_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN, $3);
	}
	| expression C_BITWISE_XOR_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN, $3);
	}
	| expression C_SHIFT_LEFT_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN, $3);
	}
	| expression C_SHIFT_RIGHT_ASSIGN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN, $3);
	}
	| expression C_LOGICAL_OR expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LOGICAL_OR, $3);
	}
	| expression C_LOGICAL_AND expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LOGICAL_AND, $3);
	}
	| expression C_BITWISE_OR expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_OR, $3);
	}
	| expression C_BITWISE_XOR expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_XOR, $3);
	}
	| expression C_BITWISE_AND expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_BITWISE_AND, $3);
	}
	| expression C_EQUAL expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_EQUAL, $3);
	}
	| expression C_NOT_EQUAL expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_NOT_EQUAL, $3);
	}
	| expression C_LESS_THAN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LESS_THAN, $3);
	}
	| expression C_GREATER_THAN expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_GREATER_THAN, $3);
	}
	| expression C_LESS_EQUAL expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_LESS_EQUAL, $3);
	}
	| expression C_GREATER_EQUAL expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_GREATER_EQUAL, $3);
	}
	| expression C_SHIFT_LEFT expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_LEFT, $3);
	}
	| expression C_SHIFT_RIGHT expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SHIFT_RIGHT, $3);
	}
	| expression C_ADD expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ADD, $3);
	}
	| expression C_SUBTRACT expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_SUBTRACT, $3);
	}
	| expression C_MULTIPLY expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MULTIPLY, $3);
	}
	| expression C_DIVIDE expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_DIVIDE, $3);
	}
	| expression C_MODULO expression {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_MODULO, $3);
	}
	| C_ADD expression %prec C_UNARY_PLUS {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_UNARY_PLUS, $2);
	}
	| C_SUBTRACT expression %prec C_UNARY_MINUS {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_UNARY_MINUS, $2);
	}
	| C_MULTIPLY expression %prec C_UNARY_DEREFERENCE {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_DEREFERENCE, $2);
	}
	| C_BITWISE_AND expression %prec C_UNARY_ADDRESS_OF {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_ADDRESS_OF, $2);
	}
	| C_LOGICAL_NOT expression %prec C_LOGICAL_NOT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_LOGICAL_NOT, $2);
	}
	| C_BITWISE_NOT expression %prec C_BITWISE_NOT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_BITWISE_NOT, $2);
	}
	| C_INCREMENT expression %prec C_PREFIX_INCREMENT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_INCREMENT, $2);
	}
	| C_DECREMENT expression %prec C_PREFIX_DECREMENT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_PREFIX_DECREMENT, $2);
	}
	| expression C_INCREMENT %prec C_POSTFIX_INCREMENT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_INCREMENT, $1);
	}
	| expression C_DECREMENT %prec C_POSTFIX_DECREMENT {
		$$ = UnaryExpressionSemanticAction(EXPRESSION_OPERATOR_POSTFIX_DECREMENT, $1);
	}
	| expression C_OPEN_BRACKET expression C_CLOSE_BRACKET %prec C_ARRAY_INDEX {
		$$ = BinaryExpressionSemanticAction($1, EXPRESSION_OPERATOR_ARRAY_INDEX, $3);
	}
	| expression C_DOT cIdentifier %prec C_MEMBER_ACCESS {
		$$ = BinaryExpressionSemanticAction(
			$1,
			EXPRESSION_OPERATOR_MEMBER_ACCESS,
			IdentifierExpressionSemanticAction($3));
	}
	| expression C_ARROW cIdentifier %prec C_MEMBER_ACCESS {
		$$ = BinaryExpressionSemanticAction(
			$1,
			EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS,
			IdentifierExpressionSemanticAction($3));
	}
	;

%%
