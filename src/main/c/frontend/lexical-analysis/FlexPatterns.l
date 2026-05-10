%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"
#include <stdbool.h>

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus InlineCBlockEndLexemeAction();
extern CompilationStatus InlineCBlockStartLexemeAction();
extern CompilationStatus InlineCContentLexemeAction();
extern bool IsInlineCClosingLine();
extern CompilationStatus IndentationLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus LineBreakLexemeAction();
extern CompilationStatus PreprocessorDirectiveLexemeAction(TokenLabel label, const char * keyword);
extern CompilationStatus PunctuationLexemeAction(TokenLabel label);
extern CompilationStatus StringLexemeAction(TokenLabel label);
extern CompilationStatus TerminatorLexemeAction();
extern CompilationStatus UnknownLexemeAction();

#define YY_USER_INIT BEGIN(BOL)

%}

/**
 * Reusable patterns.
 *
 * @see https://westes.github.io/flex/manual/Matching.html
 * @see https://westes.github.io/flex/manual/Patterns.html
 */
/* whitespace			[ \t\r\n]+ */
integer				[0-9]+
string				\"([^\\\n\"]|\\.)*\"
whitespace			[ \t\r]+
terminator			[;]+
linebreak			\n
identifier          [_A-Za-z][_A-Za-z0-9]*
includeDirective	"#include"{whitespace}+{identifier}
defineDirective		"#define"{whitespace}+[^ \t\r\n;][^;\n]*

%x BOL AFTER_POSTFIX INLINE_C BLOCK_COMMENT

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

<BOL>[ \t]*{linebreak}				{ return LineBreakLexemeAction(); }
<BOL>[ \t]*[^ \t\n]					{ CompilationStatus status = IndentationLexemeAction(); yyless(0); BEGIN(INITIAL); return status; }
<BOL><<EOF>>						{ return EOFLexemeAction(); }

<AFTER_POSTFIX>"->"/{identifier}	{ BEGIN(INITIAL); return PunctuationLexemeAction(PTR_ARROW); }
<AFTER_POSTFIX>{whitespace}			{ BEGIN(INITIAL); return IgnoredLexemeAction(); }
<AFTER_POSTFIX>"//"[^\n]*			{ BEGIN(INITIAL); return IgnoredLexemeAction(); }
<AFTER_POSTFIX>"/*"					{ BEGIN(BLOCK_COMMENT); return IgnoredLexemeAction(); }
<AFTER_POSTFIX>{linebreak}			{ BEGIN(INITIAL); yyless(0); }
<AFTER_POSTFIX><<EOF>>				{ BEGIN(INITIAL); return EOFLexemeAction(); }
<AFTER_POSTFIX>.					{ BEGIN(INITIAL); yyless(0); }

<BLOCK_COMMENT>"*/"					{ BEGIN(INITIAL); return IgnoredLexemeAction(); }
<BLOCK_COMMENT>{linebreak}			{ return LineBreakLexemeAction(); }
<BLOCK_COMMENT>[^*\n]+				{ return IgnoredLexemeAction(); }
<BLOCK_COMMENT>"*"					{ return IgnoredLexemeAction(); }
<BLOCK_COMMENT><<EOF>>				{ return UnknownLexemeAction(); }

<INLINE_C>.*{linebreak}				{ if (IsInlineCClosingLine()) { BEGIN(BOL); return InlineCBlockEndLexemeAction(); } return InlineCContentLexemeAction(); }
<INLINE_C>.+						{ if (IsInlineCClosingLine()) { BEGIN(BOL); return InlineCBlockEndLexemeAction(); } return InlineCContentLexemeAction(); }
<INLINE_C><<EOF>>					{ return UnknownLexemeAction(); }

{includeDirective}					{ return PreprocessorDirectiveLexemeAction(INCLUDE_DIRECTIVE, "#include"); }
{defineDirective}					{ return PreprocessorDirectiveLexemeAction(DEFINE_DIRECTIVE, "#define"); }
"fn*"								{ return KeywordLexemeAction(FUNCTION_POINTER); }
"fn"								{ return KeywordLexemeAction(FUNCTION); }
"ret"								{ return KeywordLexemeAction(RETURN); }
"if"								{ return KeywordLexemeAction(IF); }
"elif"								{ return KeywordLexemeAction(ELIF); }
"else"								{ return KeywordLexemeAction(ELSE); }
"ford"								{ return KeywordLexemeAction(FORD); }
"for"								{ return KeywordLexemeAction(FOR); }
"while"								{ return KeywordLexemeAction(WHILE); }
"dw"								{ return KeywordLexemeAction(DO_WHILE); }
"switch"							{ return KeywordLexemeAction(SWITCH); }
"default"							{ return KeywordLexemeAction(DEFAULT); }
"break"								{ return KeywordLexemeAction(BREAK); }
"main"								{ return KeywordLexemeAction(MAIN); }
"struct"							{ return KeywordLexemeAction(STRUCT); }
"enum"								{ return KeywordLexemeAction(ENUM); }
"union"								{ return KeywordLexemeAction(UNION); }
"typedef"							{ return KeywordLexemeAction(TYPEDEF); }
"int"								{ return KeywordLexemeAction(TYPE_INT); }
"char"								{ return KeywordLexemeAction(TYPE_CHAR); }
"float"							    { return KeywordLexemeAction(TYPE_FLOAT); }
"double"							{ return KeywordLexemeAction(TYPE_DOUBLE); }
"void"								{ return KeywordLexemeAction(TYPE_VOID); }
"uint"								{ return KeywordLexemeAction(TYPE_UINT); }
"uli"								{ return KeywordLexemeAction(TYPE_ULI); }
"long"								{ return KeywordLexemeAction(TYPE_LONG); }
"null"								{ return KeywordLexemeAction(NULL_LITERAL); }
{integer}							{ return StringLexemeAction(INTEGER_LITERAL); }
{string}							{ return StringLexemeAction(STRING_LITERAL); }
{identifier}				        { BEGIN(AFTER_POSTFIX); return IdentifierLexemeAction(); }
"//"[^\n]*							{ return IgnoredLexemeAction(); }
"/*"								{ BEGIN(BLOCK_COMMENT); return IgnoredLexemeAction(); }
"->"								{ return PunctuationLexemeAction(ARROW); }
">>="								{ return PunctuationLexemeAction(SHIFT_RIGHT_ASSIGN); }
"<<="								{ return PunctuationLexemeAction(SHIFT_LEFT_ASSIGN); }
"+="								{ return PunctuationLexemeAction(ADD_ASSIGN); }
"-="								{ return PunctuationLexemeAction(SUBTRACT_ASSIGN); }
"*="								{ return PunctuationLexemeAction(MULTIPLY_ASSIGN); }
"/="								{ return PunctuationLexemeAction(DIVIDE_ASSIGN); }
"%="								{ return PunctuationLexemeAction(MODULO_ASSIGN); }
"&="								{ return PunctuationLexemeAction(BITWISE_AND_ASSIGN); }
"|="								{ return PunctuationLexemeAction(BITWISE_OR_ASSIGN); }
"^="								{ return PunctuationLexemeAction(BITWISE_XOR_ASSIGN); }
"++"								{ return PunctuationLexemeAction(INCREMENT); }
"--"								{ return PunctuationLexemeAction(DECREMENT); }
"=="								{ return PunctuationLexemeAction(EQUAL); }
"!="								{ return PunctuationLexemeAction(NOT_EQUAL); }
"<="								{ return PunctuationLexemeAction(LESS_EQUAL); }
">="								{ return PunctuationLexemeAction(GREATER_EQUAL); }
"&&"								{ return PunctuationLexemeAction(LOGICAL_AND); }
"||"								{ return PunctuationLexemeAction(LOGICAL_OR); }
"<<"								{ return PunctuationLexemeAction(SHIFT_LEFT); }
">>"								{ return PunctuationLexemeAction(SHIFT_RIGHT); }
"="									{ return PunctuationLexemeAction(ASSIGN); }
"+"									{ return PunctuationLexemeAction(ADD); }
"-"									{ return PunctuationLexemeAction(SUBTRACT); }
"*"									{ return PunctuationLexemeAction(MULTIPLY); }
"/"									{ return PunctuationLexemeAction(DIVIDE); }
"%"									{ return PunctuationLexemeAction(MODULO); }
"!"									{ return PunctuationLexemeAction(LOGICAL_NOT); }
"~"									{ return PunctuationLexemeAction(BITWISE_NOT); }
"&"									{ return PunctuationLexemeAction(BITWISE_AND); }
"|"									{ return PunctuationLexemeAction(BITWISE_OR); }
"^"									{ return PunctuationLexemeAction(BITWISE_XOR); }
"<"									{ return PunctuationLexemeAction(LESS_THAN); }
">"									{ return PunctuationLexemeAction(GREATER_THAN); }
"."									{ return PunctuationLexemeAction(DOT); }
":"									{ return PunctuationLexemeAction(COLON); }
","									{ return PunctuationLexemeAction(COMMA); }
{terminator}						{ return TerminatorLexemeAction(); }
{linebreak}							{ BEGIN(BOL); return LineBreakLexemeAction(); }
"`"[ \t\r]*{linebreak}				{ BEGIN(INLINE_C); return InlineCBlockStartLexemeAction(); }
"("									{ return PunctuationLexemeAction(OPEN_PARENTHESIS); }
")"									{ BEGIN(AFTER_POSTFIX); return PunctuationLexemeAction(CLOSE_PARENTHESIS); }
"["									{ return PunctuationLexemeAction(OPEN_BRACKET); }
"]"									{ BEGIN(AFTER_POSTFIX); return PunctuationLexemeAction(CLOSE_BRACKET); }
"{"									{ return PunctuationLexemeAction(OPEN_BRACE); }
"}"									{ return PunctuationLexemeAction(CLOSE_BRACE); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
