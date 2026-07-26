%top{
#define YYSTYPE C_YYSTYPE
#define YYLTYPE C_YYLTYPE
#include "../CFrontend.h"
#include "../syntactic-analysis/CParser.h"
#include <stdlib.h>
#include <string.h>
}

%{
#define YY_USER_ACTION \
	yylloc->first_line = yylineno; \
	yylloc->last_line = yylineno; \
	yylloc->first_column = 1; \
	yylloc->last_column = (int) yyleng;

static char * copyLexeme(const char * text) {
	return strdup(text);
}

static char * copyInteger(const char * text) {
	if (text[0] == '0' && text[1] >= '0' && text[1] <= '7') {
		size_t length = strlen(text);
		char * converted = calloc(length + 2, sizeof(char));
		converted[0] = '0';
		converted[1] = 'o';
		memcpy(converted + 2, text + 1, length);
		return converted;
	}
	return copyLexeme(text);
}

static char * copyIncludeName(const char * text) {
	const char * start = strchr(text, '<');
	const char * suffix = strstr(start != NULL ? start : text, ".h>");
	if (start == NULL || suffix == NULL || suffix <= start + 1) {
		return NULL;
	}
	size_t length = (size_t) (suffix - start - 1);
	char * name = calloc(length + 1, sizeof(char));
	memcpy(name, start + 1, length);
	return name;
}

static char * copyDefineBody(const char * text) {
	const char * body = strstr(text, "define");
	body = body != NULL ? body + strlen("define") : text;
	while (*body == ' ' || *body == '\t') {
		body++;
	}
	size_t length = strlen(body);
	while (length > 0 && (body[length - 1] == ' ' || body[length - 1] == '\t')) {
		length--;
	}
	char * value = calloc(length + 1, sizeof(char));
	memcpy(value, body, length);
	return value;
}

static char * copyInlineC(const char * text) {
	const char * beginMarker = "/*__TEL_INLINE_C_BEGIN__*/";
	const char * endMarker = "/*__TEL_INLINE_C_END__*/";
	const char * start = text + strlen(beginMarker);
	if (*start == '\r') {
		start++;
	}
	if (*start == '\n') {
		start++;
	}
	const char * end = strstr(start, endMarker);
	const char * reservedBegin = strstr(start, "__TEL_INLINE_C_BEGIN__");
	const char * reservedEnd = strstr(start, "__TEL_INLINE_C_END__");
	if (end == NULL
		|| (reservedBegin != NULL && reservedBegin < end)
		|| (reservedEnd != NULL && reservedEnd < end)) {
		return NULL;
	}
	while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
		end--;
	}
	size_t maximumLength = (size_t) (end - start) + 2;
	char * code = calloc(maximumLength, sizeof(char));
	size_t outputLength = 0;
	for (const char * cursor = start; cursor < end; cursor++) {
		if (*cursor == '\r') {
			if (cursor + 1 < end && cursor[1] == '\n') {
				continue;
			}
			code[outputLength++] = '\n';
		}
		else {
			code[outputLength++] = *cursor;
		}
	}
	while (outputLength > 0 && code[outputLength - 1] == '\n') {
		outputLength--;
	}
	code[outputLength++] = '\n';
	code[outputLength] = '\0';
	return code;
}
%}

%option extra-type="CFrontendContext *"

identifier		[_A-Za-z][_A-Za-z0-9]*
hexInteger		0[xX][0-9a-fA-F]+
octalInteger	0[0-7]+
integer			(0|[1-9][0-9]*)
float			([0-9]+\.[0-9]*([eE][+-]?[0-9]+)?|\.[0-9]+([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+)
charLiteral		'([^\\'\n]|\\(x[0-9a-fA-F]{1,2}|.))'
stringLiteral	\"([^\\\n\"]|\\.)*\"
includeDirective	#[ \t]*include[ \t]*<[_A-Za-z][_A-Za-z0-9]*\.h>
defineDirective		#[ \t]*define[ \t]+[^\r\n]+
generatedNull		\(\([ \t]*void[ \t]*\*[ \t]*\)[ \t]*0[ \t]*\)

%x BLOCK_COMMENT INLINE_C_MARKER

%%

[ \t\r\n]+					{ /* discard formatting */ }
"//"[^\n]*					{ /* discard ordinary comments */ }
"/*__TEL_INLINE_C_BEGIN__*/"	{ BEGIN(INLINE_C_MARKER); yymore(); }
<INLINE_C_MARKER>"/*__TEL_INLINE_C_END__*/" {
								yylval->string = copyInlineC(yytext);
								BEGIN(INITIAL);
								if (yylval->string == NULL) {
									return C_YYUNDEF;
								}
								return C_INLINE_C_BLOCK;
							}
<INLINE_C_MARKER>\n			{ yymore(); }
<INLINE_C_MARKER>.			{ yymore(); }
<INLINE_C_MARKER><<EOF>>	{ return C_YYUNDEF; }
"/*"						{ BEGIN(BLOCK_COMMENT); }
<BLOCK_COMMENT>"*/"			{ BEGIN(INITIAL); }
<BLOCK_COMMENT>[^*\n]+		{ /* discard */ }
<BLOCK_COMMENT>"*"			{ /* discard */ }
<BLOCK_COMMENT>\n			{ /* yylineno is maintained by flex */ }
<BLOCK_COMMENT><<EOF>>		{ return C_YYUNDEF; }

{includeDirective}			{ yylval->string = copyIncludeName(yytext); return C_INCLUDE_DIRECTIVE; }
{defineDirective}			{ yylval->string = copyDefineBody(yytext); return C_DEFINE_DIRECTIVE; }

"int"						{ return C_INT; }
"char"						{ return C_CHAR; }
"float"					{ return C_FLOAT; }
"double"					{ return C_DOUBLE; }
"void"						{ return C_VOID; }
"unsigned"					{ return C_UNSIGNED; }
"long"						{ return C_LONG; }
"short"					{ return C_SHORT; }
"const"					{ return C_CONST; }
"static"					{ return C_STATIC; }
"typedef"					{ return C_TYPEDEF; }
"struct"					{ return C_STRUCT; }
"union"						{ return C_UNION; }
"enum"						{ return C_ENUM; }
"return"					{ return C_RETURN; }
"if"						{ return C_IF; }
"else"						{ return C_ELSE; }
"for"						{ return C_FOR; }
"while"					{ return C_WHILE; }
"do"						{ return C_DO; }
"switch"					{ return C_SWITCH; }
"case"						{ return C_CASE; }
"default"					{ return C_DEFAULT; }
"break"					{ return C_BREAK; }
"continue"					{ return C_CONTINUE; }
"NULL"						{ return C_NULL; }
{generatedNull}				{ return C_NULL; }

{hexInteger}				{ yylval->string = copyLexeme(yytext); return C_INTEGER_LITERAL; }
{octalInteger}				{ yylval->string = copyInteger(yytext); return C_INTEGER_LITERAL; }
{float}						{ yylval->string = copyLexeme(yytext); return C_FLOAT_LITERAL; }
{integer}					{ yylval->string = copyLexeme(yytext); return C_INTEGER_LITERAL; }
{charLiteral}				{ yylval->string = copyLexeme(yytext); return C_CHAR_LITERAL; }
{stringLiteral}				{ yylval->string = copyLexeme(yytext); return C_STRING_LITERAL; }
{identifier}				{ yylval->string = copyLexeme(yytext); return isCFrontendTypedefName(yyextra, yytext) ? C_TYPEDEF_NAME : C_IDENTIFIER; }

">>="						{ return C_SHIFT_RIGHT_ASSIGN; }
"<<="						{ return C_SHIFT_LEFT_ASSIGN; }
"+="						{ return C_ADD_ASSIGN; }
"-="						{ return C_SUBTRACT_ASSIGN; }
"*="						{ return C_MULTIPLY_ASSIGN; }
"/="						{ return C_DIVIDE_ASSIGN; }
"%="						{ return C_MODULO_ASSIGN; }
"&="						{ return C_BITWISE_AND_ASSIGN; }
"|="						{ return C_BITWISE_OR_ASSIGN; }
"^="						{ return C_BITWISE_XOR_ASSIGN; }
"++"						{ return C_INCREMENT; }
"--"						{ return C_DECREMENT; }
"=="						{ return C_EQUAL; }
"!="						{ return C_NOT_EQUAL; }
"<="						{ return C_LESS_EQUAL; }
">="						{ return C_GREATER_EQUAL; }
"&&"						{ return C_LOGICAL_AND; }
"||"						{ return C_LOGICAL_OR; }
"<<"						{ return C_SHIFT_LEFT; }
">>"						{ return C_SHIFT_RIGHT; }
"->"						{ return C_ARROW; }
"="						{ return C_ASSIGN; }
"+"						{ return C_ADD; }
"-"						{ return C_SUBTRACT; }
"*"						{ return C_MULTIPLY; }
"/"						{ return C_DIVIDE; }
"%"						{ return C_MODULO; }
"!"						{ return C_LOGICAL_NOT; }
"~"						{ return C_BITWISE_NOT; }
"&"						{ return C_BITWISE_AND; }
"|"						{ return C_BITWISE_OR; }
"^"						{ return C_BITWISE_XOR; }
"<"						{ return C_LESS_THAN; }
">"						{ return C_GREATER_THAN; }
"."						{ return C_DOT; }
"("						{ return C_OPEN_PARENTHESIS; }
")"						{ return C_CLOSE_PARENTHESIS; }
"["						{ return C_OPEN_BRACKET; }
"]"						{ return C_CLOSE_BRACKET; }
"{"						{ pushCFrontendScope(yyextra); return C_OPEN_BRACE; }
"}"						{ popCFrontendScope(yyextra); return C_CLOSE_BRACE; }
","						{ return C_COMMA; }
":"						{ return C_COLON; }
";"						{ return C_SEMICOLON; }

.							{ return C_YYUNDEF; }

%%
