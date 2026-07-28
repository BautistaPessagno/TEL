#define _GNU_SOURCE
#include "backend/code-generation/CodeGenerator.h"
#include "backend/normalization/ProgramNormalizer.h"
#include "backend/semantic-analysis/SemanticAnalyzer.h"
#include "backend/tel-code-generation/TelCodeGenerator.h"
#include "frontend/Frontend.h"
#include "frontend/c-language/CFrontend.h"
#include "frontend/lexical-analysis/FlexActions.h"
#include "frontend/syntactic-analysis/BisonActions.h"
#include "support/configuration/CompilerOptions.h"
#include "support/logging/Logger.h"
#include "support/type/CompilationStatus.h"
#include "support/type/CompilerState.h"
#include "support/type/ModuleDestructor.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static CompilationStatus _writeArtifact(
	const char * artifact,
	size_t artifactLength,
	const char * outputPath,
	Logger * logger) {
	if (outputPath == NULL) {
		if (fwrite(artifact, 1, artifactLength, stdout) != artifactLength
			|| fflush(stdout) != 0) {
			logError(logger, "Failed to write generated output.");
			return FAILED;
		}
		return SUCCEEDED;
	}
	size_t templateLength = strlen(outputPath) + strlen(".tmp.XXXXXX") + 1;
	char * temporaryPath = calloc(templateLength, sizeof(char));
	if (temporaryPath == NULL) {
		return OUT_OF_MEMORY;
	}
	snprintf(temporaryPath, templateLength, "%s.tmp.XXXXXX", outputPath);
	int descriptor = mkstemp(temporaryPath);
	if (descriptor < 0) {
		logError(logger, "Cannot create output file %s: %s", outputPath, strerror(errno));
		free(temporaryPath);
		return FAILED;
	}
	FILE * output = fdopen(descriptor, "w");
	bool succeeded = output != NULL;
	if (output == NULL) {
		close(descriptor);
	}
	else {
		if (fwrite(artifact, 1, artifactLength, output) != artifactLength
			|| fflush(output) != 0) {
			succeeded = false;
		}
		if (fclose(output) != 0) {
			succeeded = false;
		}
		if (succeeded && rename(temporaryPath, outputPath) != 0) {
			succeeded = false;
		}
	}
	if (!succeeded) {
		logError(logger, "Cannot write output file %s: %s", outputPath, strerror(errno));
		unlink(temporaryPath);
	}
	free(temporaryPath);
	return succeeded ? SUCCEEDED : FAILED;
}

static CompilationStatus _generateArtifact(
	CompilerState * compilerState,
	const CompilerOptions * options,
	Logger * logger) {
	char * artifact = NULL;
	size_t artifactLength = 0;
	FILE * stream = open_memstream(&artifact, &artifactLength);
	if (stream == NULL) {
		logError(logger, "Cannot allocate generated output.");
		return OUT_OF_MEMORY;
	}
	CompilationStatus status = options->sourceLanguage == SOURCE_LANGUAGE_TEL
		? executeCodeGeneration(compilerState, stream)
		: executeTelCodeGeneration(compilerState, stream);
	if (fclose(stream) != 0) {
		status = FAILED;
	}
	if (status == SUCCEEDED) {
		status = _writeArtifact(artifact, artifactLength, options->outputPath, logger);
	}
	free(artifact);
	return status;
}

const int main(const int argumentCount, const char ** arguments) {
	CompilerOptions options;
	if (!parseCompilerOptions(argumentCount, arguments, &options, stderr)) {
		printCompilerUsage(stderr, arguments[0]);
		return FAILED;
	}
	if (options.showHelp) {
		printCompilerUsage(stdout, arguments[0]);
		return SUCCEEDED;
	}
	FILE * input = stdin;
	if (options.inputPath != NULL) {
		input = fopen(options.inputPath, "r");
		if (input == NULL) {
			fprintf(stderr, "Cannot open input file %s: %s\n", options.inputPath, strerror(errno));
			return FAILED;
		}
	}

	Logger * logger = createLogger("EntryPoint");
	CompilerState compilerState = {.abstractSyntaxtTree = NULL};
	ModuleDestructor moduleDestructors[6];
	size_t moduleDestructorCount = 0;
	moduleDestructors[moduleDestructorCount++] = initializeAbstractSyntaxTreeModule();
	moduleDestructors[moduleDestructorCount++] = initializeSemanticAnalyzerModule();
	moduleDestructors[moduleDestructorCount++] = initializeCodeGeneratorModule();
	LexicalAnalyzer * lexicalAnalyzer = NULL;
	CompilationStatus status;
	if (options.sourceLanguage == SOURCE_LANGUAGE_TEL) {
		lexicalAnalyzer = createLexicalAnalyzer();
		setLexicalAnalyzerInput(lexicalAnalyzer, input);
		moduleDestructors[moduleDestructorCount++] = initializeFlexActionsModule(lexicalAnalyzer);
		moduleDestructors[moduleDestructorCount++] = initializeBisonActionsModule(&compilerState);
		moduleDestructors[moduleDestructorCount++] = initializeFrontendModule(lexicalAnalyzer);
		SetBisonSourceName(options.inputPath != NULL ? options.inputPath : "<stdin>");
		status = executeSyntacticAnalysis();
	}
	else {
		moduleDestructors[moduleDestructorCount++] = initializeBisonActionsModule(&compilerState);
		const char * sourceName = options.inputPath != NULL ? options.inputPath : "<stdin>";
		status = executeCFrontend(input, sourceName, &compilerState);
	}
	Program * program = compilerState.abstractSyntaxtTree;
	if (status == SUCCEEDED) {
		status = executeSemanticAnalysis(&compilerState);
		if (status != SUCCEEDED) {
			const char * sourceName = options.inputPath != NULL ? options.inputPath : "<stdin>";
			fprintf(stderr, "%s:1: semantic validation failed\n", sourceName);
		}
	}
	if (status == SUCCEEDED) {
		status = normalizeProgram(&compilerState);
	}
	if (status == SUCCEEDED) {
		status = _generateArtifact(&compilerState, &options, logger);
	}
	else {
		logError(logger, "The input program was rejected.");
	}

	destroyProgram(program);
	for (int index = (int) moduleDestructorCount - 1;
		index >= 0;
		index--) {
		moduleDestructors[index]();
	}
	destroyLogger(logger);
	if (lexicalAnalyzer != NULL) {
		destroyLexicalAnalyzer(lexicalAnalyzer);
	}
	if (input != stdin) {
		fclose(input);
	}
	return status;
}
