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
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus PunctuationLexemeAction(TokenLabel label);
extern CompilationStatus StringLexemeAction(TokenLabel label);
extern CompilationStatus TerminatorLexemeAction();
extern CompilationStatus UnknownLexemeAction();

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
terminator			[;\n]+

%%

%{

/**
 * @see https://westes.github.io/flex/manual/Rules-Section.html#Rules-Section
 */

%}

"fn"								{ return KeywordLexemeAction(FUNCTION); }
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
[_A-Za-z][_A-Za-z0-9]*				{ return IdentifierLexemeAction(); }
"->"								{ return PunctuationLexemeAction(ARROW); }
":"									{ return PunctuationLexemeAction(COLON); }
{terminator}						{ return TerminatorLexemeAction(); }
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
