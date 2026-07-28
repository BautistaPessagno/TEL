#ifndef COMPILER_OPTIONS_HEADER
#define COMPILER_OPTIONS_HEADER

#include <stdbool.h>
#include <stdio.h>

typedef enum {
	SOURCE_LANGUAGE_TEL,
	SOURCE_LANGUAGE_C
} SourceLanguage;

typedef struct {
	const char * inputPath;
	const char * outputPath;
	SourceLanguage sourceLanguage;
	bool sourceLanguageWasExplicit;
	bool showHelp;
} CompilerOptions;

bool parseCompilerOptions(
	int argumentCount,
	const char ** arguments,
	CompilerOptions * options,
	FILE * errors);

void printCompilerUsage(FILE * output, const char * executableName);

#endif
