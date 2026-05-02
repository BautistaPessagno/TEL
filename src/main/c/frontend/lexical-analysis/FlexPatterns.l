%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

/**
 * @see https://westes.github.io/flex/manual/Definitions-Section.html#Definitions-Section
 */

extern CompilationStatus EOFLexemeAction();
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus IndentationLexemeAction();
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus LineBreakLexemeAction();
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

%x BOL

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

<BOL>[ \t]*{linebreak}				{ return LineBreakLexemeAction(); }
<BOL>[ \t]*[^ \t\n]					{ CompilationStatus status = IndentationLexemeAction(); yyless(0); BEGIN(INITIAL); return status; }
<BOL><<EOF>>						{ return EOFLexemeAction(); }

"fn"								{ return KeywordLexemeAction(FUNCTION); }
"ret"								{ return KeywordLexemeAction(RETURN); }
"main"								{ return KeywordLexemeAction(MAIN); }
"int"								{ return KeywordLexemeAction(TYPE_INT); }
"char"								{ return KeywordLexemeAction(TYPE_CHAR); }
"float"							    { return KeywordLexemeAction(TYPE_FLOAT); }
"double"							{ return KeywordLexemeAction(TYPE_DOUBLE); }
"void"								{ return KeywordLexemeAction(TYPE_VOID); }
"uint"								{ return KeywordLexemeAction(TYPE_UINT); }
"uli"								{ return KeywordLexemeAction(TYPE_ULI); }
{integer}							{ return StringLexemeAction(INTEGER_LITERAL); }
{string}							{ return StringLexemeAction(STRING_LITERAL); }
{identifier}				        { return IdentifierLexemeAction(); }
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
":"									{ return PunctuationLexemeAction(COLON); }
{terminator}						{ return TerminatorLexemeAction(); }
{linebreak}							{ BEGIN(BOL); return LineBreakLexemeAction(); }
"("									{ return PunctuationLexemeAction(OPEN_PARENTHESIS); }
")"									{ return PunctuationLexemeAction(CLOSE_PARENTHESIS); }

{whitespace}						{ return IgnoredLexemeAction(); }
.									{ return UnknownLexemeAction(); }

<<EOF>>								{ return EOFLexemeAction(); }

%%

/**
 * @see https://westes.github.io/flex/manual/User-Code-Section.html#User-Code-Section
 */

#include "FlexActions.h"
#include "FlexExport.h"
