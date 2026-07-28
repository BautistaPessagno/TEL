#include "CompilerOptions.h"
#include <string.h>

static bool _hasExtension(const char * path, const char * extension) {
	if (path == NULL || extension == NULL) {
		return false;
	}
	size_t pathLength = strlen(path);
	size_t extensionLength = strlen(extension);
	return pathLength >= extensionLength
		&& strcmp(path + pathLength - extensionLength, extension) == 0;
}

static bool _parseSourceLanguage(
	const char * value,
	SourceLanguage * sourceLanguage,
	FILE * errors) {
	if (strcmp(value, "tel") == 0) {
		*sourceLanguage = SOURCE_LANGUAGE_TEL;
		return true;
	}
	if (strcmp(value, "c") == 0) {
		*sourceLanguage = SOURCE_LANGUAGE_C;
		return true;
	}
	fprintf(errors, "Unknown source language: %s\n", value);
	return false;
}

bool parseCompilerOptions(
	int argumentCount,
	const char ** arguments,
	CompilerOptions * options,
	FILE * errors) {
	if (options == NULL || errors == NULL) {
		return false;
	}
	*options = (CompilerOptions) {
		.sourceLanguage = SOURCE_LANGUAGE_TEL
	};
	for (int index = 1; index < argumentCount; index++) {
		const char * argument = arguments[index];
		if (strcmp(argument, "--help") == 0) {
			if (options->showHelp) {
				fprintf(errors, "--help may only be specified once.\n");
				return false;
			}
			options->showHelp = true;
			continue;
		}
		if (strcmp(argument, "--from") == 0) {
			if (options->sourceLanguageWasExplicit) {
				fprintf(errors, "--from may only be specified once.\n");
				return false;
			}
			if (++index >= argumentCount) {
				fprintf(errors, "--from requires tel or c.\n");
				return false;
			}
			if (!_parseSourceLanguage(arguments[index], &options->sourceLanguage, errors)) {
				return false;
			}
			options->sourceLanguageWasExplicit = true;
			continue;
		}
		if (strcmp(argument, "-o") == 0) {
			if (options->outputPath != NULL) {
				fprintf(errors, "-o may only be specified once.\n");
				return false;
			}
			if (++index >= argumentCount) {
				fprintf(errors, "-o requires an output path.\n");
				return false;
			}
			options->outputPath = arguments[index];
			continue;
		}
		if (argument[0] == '-') {
			fprintf(errors, "Unknown option: %s\n", argument);
			return false;
		}
		if (options->inputPath != NULL) {
			fprintf(errors, "Only one input file may be specified.\n");
			return false;
		}
		options->inputPath = argument;
	}
	if (options->showHelp) {
		if (options->inputPath != NULL
			|| options->outputPath != NULL
			|| options->sourceLanguageWasExplicit) {
			fprintf(errors, "--help cannot be combined with compilation options.\n");
			return false;
		}
		return true;
	}
	if (options->inputPath != NULL) {
		if (options->sourceLanguageWasExplicit) {
			fprintf(errors, "--from cannot be combined with a named input file.\n");
			return false;
		}
		if (_hasExtension(options->inputPath, ".tel")) {
			options->sourceLanguage = SOURCE_LANGUAGE_TEL;
		}
		else if (_hasExtension(options->inputPath, ".c")) {
			options->sourceLanguage = SOURCE_LANGUAGE_C;
		}
		else {
			fprintf(errors, "Input file must have a lowercase .tel or .c extension.\n");
			return false;
		}
	}
	return true;
}

void printCompilerUsage(FILE * output, const char * executableName) {
	const char * name = executableName != NULL ? executableName : "tel";
	fprintf(output,
		"Usage:\n"
		"  %s [program.tel|program.c] [-o output]\n"
		"  %s --from tel|c [-o output] < source\n"
		"  %s --help\n",
		name,
		name,
		name);
}
