/* _GNU_SOURCE exposes open_memstream(), used to capture sub-expressions and
 * parameter lists into strings. It is available on Linux (the documented
 * container build target) and modern macOS/BSD libc. */
#define _GNU_SOURCE
#include "CodeGenerator.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

typedef struct {
	FILE * output;
	unsigned int indentation;
	bool failed;
} CodeGenerationContext;

typedef bool (*ProgramItemPredicate)(ProgramItem * item);

static void _emit(CodeGenerationContext * context, const char * format, ...);
static void _emitIndentation(CodeGenerationContext * context);
static void _emitLine(CodeGenerationContext * context, const char * format, ...);
static char * _format(const char * format, ...);
static char * _captureExpression(Expression * expression);
static char * _captureParameters(ParameterList * parameters);
static char * _buildDeclarator(Type * type, const char * name, Type ** baseType);
static const char * _baseTypeName(Type * type);
static const char * _binaryOperator(ExpressionOperator operator);
static const char * _unaryOperator(ExpressionOperator operator);
static void _generateProgram(CodeGenerationContext * context, Program * program);
static void _generateMatchingItems(
	CodeGenerationContext * context,
	Program * program,
	ProgramItemPredicate predicate);
static void _generateFunctionPrototypes(CodeGenerationContext * context, Program * program);
static void _generateProgramItem(CodeGenerationContext * context, ProgramItem * item);
static void _generatePreprocessorDirective(CodeGenerationContext * context, PreprocessorDirective * directive);
static void _generateInlineC(CodeGenerationContext * context, const char * code);
static void _generateTypedef(CodeGenerationContext * context, TypedefDeclaration * declaration);
static void _generateEnum(CodeGenerationContext * context, EnumDeclaration * declaration);
static void _generateAggregate(CodeGenerationContext * context, AggregateDeclaration * declaration);
static void _generateVariableDeclaration(
	CodeGenerationContext * context,
	VariableDeclaration * declaration,
	bool includeTerminator);
static void _generateFunctionPrototype(CodeGenerationContext * context, FunctionDeclaration * declaration);
static void _generateFunctionDefinition(CodeGenerationContext * context, FunctionDeclaration * declaration);
static void _generateMain(CodeGenerationContext * context, MainDeclaration * declaration);
static void _generateTypeDeclaration(CodeGenerationContext * context, Type * type, const char * name);
static void _generateParameters(CodeGenerationContext * context, ParameterList * parameters);
static void _generateBlock(CodeGenerationContext * context, StatementList * statements);
static void _generateStatementList(CodeGenerationContext * context, StatementList * statements);
static void _generateStatement(CodeGenerationContext * context, Statement * statement);
static void _generateIf(CodeGenerationContext * context, IfStatement * statement);
static void _generateFor(CodeGenerationContext * context, ForStatement * statement);
static void _generateWhile(CodeGenerationContext * context, WhileStatement * statement);
static void _generateDoWhile(CodeGenerationContext * context, DoWhileStatement * statement);
static void _generateSwitch(CodeGenerationContext * context, SwitchStatement * statement);
static void _generateExpression(CodeGenerationContext * context, Expression * expression);
static void _generateExpressionList(CodeGenerationContext * context, ExpressionList * expressions, const char * separator);
static void _generateIntegerLiteral(CodeGenerationContext * context, const char * value);
static bool _isDirective(ProgramItem * item);
static bool _isGlobalInlineC(ProgramItem * item);
static bool _isEnum(ProgramItem * item);
static bool _isAggregate(ProgramItem * item);
static bool _isTypedef(ProgramItem * item);
static bool _isGlobalVariable(ProgramItem * item);
static bool _isFunctionDefinition(ProgramItem * item);
static bool _isMain(ProgramItem * item);

/** Shutdown module's internal state. */
void _shutdownCodeGeneratorModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: CodeGenerator...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeCodeGeneratorModule() {
	_logger = createLogger("CodeGenerator");
	return _shutdownCodeGeneratorModule;
}

static void _emit(CodeGenerationContext * context, const char * format, ...) {
	if (context == NULL || context->failed) {
		return;
	}
	va_list arguments;
	va_start(arguments, format);
	if (vfprintf(context->output, format, arguments) < 0) {
		context->failed = true;
	}
	va_end(arguments);
}

static void _emitIndentation(CodeGenerationContext * context) {
	for (unsigned int index = 0; index < context->indentation; index++) {
		_emit(context, "    ");
	}
}

static void _emitLine(CodeGenerationContext * context, const char * format, ...) {
	if (context == NULL || context->failed) {
		return;
	}
	_emitIndentation(context);
	va_list arguments;
	va_start(arguments, format);
	if (vfprintf(context->output, format, arguments) < 0) {
		context->failed = true;
	}
	va_end(arguments);
	_emit(context, "\n");
}

static char * _format(const char * format, ...) {
	va_list arguments;
	va_start(arguments, format);
	va_list copy;
	va_copy(copy, arguments);
	int length = vsnprintf(NULL, 0, format, copy);
	va_end(copy);
	if (length < 0) {
		va_end(arguments);
		return NULL;
	}
	char * result = calloc((size_t) length + 1, sizeof(char));
	if (result != NULL) {
		vsnprintf(result, (size_t) length + 1, format, arguments);
	}
	va_end(arguments);
	return result;
}

static char * _captureExpression(Expression * expression) {
	char * buffer = NULL;
	size_t length = 0;
	FILE * stream = open_memstream(&buffer, &length);
	if (stream == NULL) {
		return NULL;
	}
	CodeGenerationContext context = { .output = stream };
	_generateExpression(&context, expression);
	if (fclose(stream) != 0 || context.failed) {
		free(buffer);
		return NULL;
	}
	return buffer;
}

static char * _captureParameters(ParameterList * parameters) {
	char * buffer = NULL;
	size_t length = 0;
	FILE * stream = open_memstream(&buffer, &length);
	if (stream == NULL) {
		return NULL;
	}
	CodeGenerationContext context = { .output = stream };
	_generateParameters(&context, parameters);
	if (fclose(stream) != 0 || context.failed) {
		free(buffer);
		return NULL;
	}
	return buffer;
}

static char * _buildDeclarator(Type * type, const char * name, Type ** baseType) {
	if (type == NULL) {
		return NULL;
	}
	switch (type->kind) {
		case TYPE_POINTER_KIND: {
			bool needsParentheses = type->pointee != NULL
				&& (type->pointee->kind == TYPE_ARRAY_KIND
					|| type->pointee->kind == TYPE_FUNCTION_POINTER_KIND);
			char * nextName = _format(needsParentheses ? "(*%s)" : "*%s", name);
			char * result = nextName != NULL
				? _buildDeclarator(type->pointee, nextName, baseType)
				: NULL;
			free(nextName);
			return result;
		}
		case TYPE_ARRAY_KIND: {
			char * size = type->arraySize != NULL ? _captureExpression(type->arraySize) : strdup("");
			char * nextName = size != NULL ? _format("%s[%s]", name, size) : NULL;
			char * result = nextName != NULL
				? _buildDeclarator(type->pointee, nextName, baseType)
				: NULL;
			free(size);
			free(nextName);
			return result;
		}
		case TYPE_FUNCTION_POINTER_KIND: {
			char * parameters = _captureParameters(type->functionParams);
			char * nextName = parameters != NULL ? _format("(*%s)(%s)", name, parameters) : NULL;
			char * result = nextName != NULL
				? _buildDeclarator(type->returnType, nextName, baseType)
				: NULL;
			free(parameters);
			free(nextName);
			return result;
		}
		default:
			*baseType = type;
			return strdup(name);
	}
}

static const char * _baseTypeName(Type * type) {
	switch (type->kind) {
		case TYPE_INT_KIND: return "int";
		case TYPE_CHAR_KIND: return "char";
		case TYPE_FLOAT_KIND: return "float";
		case TYPE_DOUBLE_KIND: return "double";
		case TYPE_VOID_KIND: return "void";
		case TYPE_UINT_KIND: return "unsigned int";
		case TYPE_ULI_KIND: return "unsigned long int";
		case TYPE_LONG_KIND: return "long";
		case TYPE_SHORT_KIND: return "short";
		case TYPE_NAMED_KIND: return type->name;
		case TYPE_STRUCT_KIND: return "struct";
		case TYPE_ENUM_KIND: return "enum";
		case TYPE_UNION_KIND: return "union";
		default: return NULL;
	}
}

static const char * _binaryOperator(ExpressionOperator operator) {
	switch (operator) {
		case EXPRESSION_OPERATOR_ASSIGN: return "=";
		case EXPRESSION_OPERATOR_ADD_ASSIGN: return "+=";
		case EXPRESSION_OPERATOR_SUBTRACT_ASSIGN: return "-=";
		case EXPRESSION_OPERATOR_MULTIPLY_ASSIGN: return "*=";
		case EXPRESSION_OPERATOR_DIVIDE_ASSIGN: return "/=";
		case EXPRESSION_OPERATOR_MODULO_ASSIGN: return "%=";
		case EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN: return "&=";
		case EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN: return "|=";
		case EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN: return "^=";
		case EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN: return "<<=";
		case EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN: return ">>=";
		case EXPRESSION_OPERATOR_LOGICAL_OR: return "||";
		case EXPRESSION_OPERATOR_LOGICAL_AND: return "&&";
		case EXPRESSION_OPERATOR_BITWISE_OR: return "|";
		case EXPRESSION_OPERATOR_BITWISE_XOR: return "^";
		case EXPRESSION_OPERATOR_BITWISE_AND: return "&";
		case EXPRESSION_OPERATOR_EQUAL: return "==";
		case EXPRESSION_OPERATOR_NOT_EQUAL: return "!=";
		case EXPRESSION_OPERATOR_LESS_THAN: return "<";
		case EXPRESSION_OPERATOR_GREATER_THAN: return ">";
		case EXPRESSION_OPERATOR_LESS_EQUAL: return "<=";
		case EXPRESSION_OPERATOR_GREATER_EQUAL: return ">=";
		case EXPRESSION_OPERATOR_SHIFT_LEFT: return "<<";
		case EXPRESSION_OPERATOR_SHIFT_RIGHT: return ">>";
		case EXPRESSION_OPERATOR_ADD: return "+";
		case EXPRESSION_OPERATOR_SUBTRACT: return "-";
		case EXPRESSION_OPERATOR_MULTIPLY: return "*";
		case EXPRESSION_OPERATOR_DIVIDE: return "/";
		case EXPRESSION_OPERATOR_MODULO: return "%";
		default: return NULL;
	}
}

static const char * _unaryOperator(ExpressionOperator operator) {
	switch (operator) {
		case EXPRESSION_OPERATOR_UNARY_PLUS: return "+";
		case EXPRESSION_OPERATOR_UNARY_MINUS: return "-";
		case EXPRESSION_OPERATOR_LOGICAL_NOT: return "!";
		case EXPRESSION_OPERATOR_BITWISE_NOT: return "~";
		case EXPRESSION_OPERATOR_DEREFERENCE: return "*";
		case EXPRESSION_OPERATOR_ADDRESS_OF: return "&";
		case EXPRESSION_OPERATOR_PREFIX_INCREMENT: return "++";
		case EXPRESSION_OPERATOR_PREFIX_DECREMENT: return "--";
		default: return NULL;
	}
}

static void _generateProgram(CodeGenerationContext * context, Program * program) {
	/* Passes emitted before function prototypes: directives, inline C, and the
	 * type declarations a prototype's signature may depend on. */
	ProgramItemPredicate beforePrototypes[] = {
		_isDirective,
		_isGlobalInlineC,
		_isEnum,
		_isAggregate,
		_isTypedef
	};
	/* Passes emitted after function prototypes: globals (whose initializers may
	 * reference a function designator), function bodies, and `main`. */
	ProgramItemPredicate afterPrototypes[] = {
		_isGlobalVariable,
		_isFunctionDefinition,
		_isMain
	};
	for (size_t index = 0; index < sizeof(beforePrototypes) / sizeof(beforePrototypes[0]); index++) {
		_generateMatchingItems(context, program, beforePrototypes[index]);
	}
	_generateFunctionPrototypes(context, program);
	for (size_t index = 0; index < sizeof(afterPrototypes) / sizeof(afterPrototypes[0]); index++) {
		_generateMatchingItems(context, program, afterPrototypes[index]);
	}
}

static void _generateMatchingItems(
	CodeGenerationContext * context,
	Program * program,
	ProgramItemPredicate predicate) {
	bool emitted = false;
	for (ProgramItemList * node = program->items; node != NULL; node = node->next) {
		if (node->item != NULL && predicate(node->item)) {
			_generateProgramItem(context, node->item);
			emitted = true;
		}
	}
	if (emitted) {
		_emit(context, "\n");
	}
}

static void _generateFunctionPrototypes(CodeGenerationContext * context, Program * program) {
	bool emitted = false;
	for (ProgramItemList * node = program->items; node != NULL; node = node->next) {
		if (node->item != NULL && node->item->kind == PROGRAM_ITEM_FUNCTION_DECLARATION) {
			_generateFunctionPrototype(context, node->item->functionDeclaration);
			emitted = true;
		}
	}
	if (emitted) {
		_emit(context, "\n");
	}
}

static void _generateProgramItem(CodeGenerationContext * context, ProgramItem * item) {
	switch (item->kind) {
		case PROGRAM_ITEM_VARIABLE_DECLARATION:
			_generateVariableDeclaration(context, item->variableDeclaration, true);
			break;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			if (item->functionDeclaration->body == NULL) {
				_generateFunctionPrototype(context, item->functionDeclaration);
			}
			else {
				_generateFunctionDefinition(context, item->functionDeclaration);
			}
			break;
		case PROGRAM_ITEM_MAIN_DECLARATION:
			_generateMain(context, item->mainDeclaration);
			break;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION:
			_generateAggregate(context, item->aggregateDeclaration);
			break;
		case PROGRAM_ITEM_ENUM_DECLARATION:
			_generateEnum(context, item->enumDeclaration);
			break;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION:
			_generateTypedef(context, item->typedefDeclaration);
			break;
		case PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE:
			_generatePreprocessorDirective(context, item->preprocessorDirective);
			break;
		case PROGRAM_ITEM_INLINE_C:
			_generateInlineC(context, item->inlineC);
			break;
		default:
			break;
	}
}

static void _generatePreprocessorDirective(CodeGenerationContext * context, PreprocessorDirective * directive) {
	if (directive->kind == PREPROCESSOR_INCLUDE_DIRECTIVE) {
		_emitLine(context, "#include <%s.h>", directive->value);
	}
	else {
		_emitLine(context, "#define %s", directive->value);
	}
}

static void _generateInlineC(CodeGenerationContext * context, const char * code) {
	if (code == NULL || *code == '\0') {
		return;
	}
	_emit(context, "%s", code);
	if (code[strlen(code) - 1] != '\n') {
		_emit(context, "\n");
	}
}

static void _generateTypedef(CodeGenerationContext * context, TypedefDeclaration * declaration) {
	_emitIndentation(context);
	_emit(context, "typedef ");
	_generateTypeDeclaration(context, declaration->type, declaration->name);
	_emit(context, ";\n");
}

static void _generateEnum(CodeGenerationContext * context, EnumDeclaration * declaration) {
	_emitLine(context, "enum %s {", declaration->name);
	context->indentation++;
	for (EnumMemberList * node = declaration->members; node != NULL; node = node->next) {
		_emitIndentation(context);
		_emit(context, "%s", node->member->name);
		if (node->member->value != NULL) {
			_emit(context, " = ");
			_generateIntegerLiteral(context, node->member->value);
		}
		if (node->next != NULL) {
			_emit(context, ",");
		}
		_emit(context, "\n");
	}
	context->indentation--;
	_emitLine(context, "};");
}

static void _generateAggregate(CodeGenerationContext * context, AggregateDeclaration * declaration) {
	const char * keyword = declaration->kind == AGGREGATE_STRUCT_KIND ? "struct" : "union";
	_emitLine(context, "%s %s {", keyword, declaration->name);
	context->indentation++;
	for (VariableDeclarationList * node = declaration->fields; node != NULL; node = node->next) {
		_generateVariableDeclaration(context, node->declaration, true);
	}
	context->indentation--;
	_emitLine(context, "};");
}

static void _generateVariableDeclaration(
	CodeGenerationContext * context,
	VariableDeclaration * declaration,
	bool includeTerminator) {
	_emitIndentation(context);
	if (declaration->isStatic) {
		_emit(context, "static ");
	}
	_generateTypeDeclaration(context, declaration->type, declaration->name);
	if (declaration->initializer != NULL) {
		_emit(context, " = ");
		_generateExpression(context, declaration->initializer);
	}
	if (includeTerminator) {
		_emit(context, ";");
	}
	_emit(context, "\n");
}

static void _generateFunctionPrototype(CodeGenerationContext * context, FunctionDeclaration * declaration) {
	_emitIndentation(context);
	if (declaration->isStatic) {
		_emit(context, "static ");
	}
	char * parameters = _captureParameters(declaration->parameters);
	char * declarator = parameters != NULL ? _format("%s(%s)", declaration->name, parameters) : NULL;
	if (declarator == NULL) {
		context->failed = true;
	}
	else {
		_generateTypeDeclaration(context, declaration->returnType, declarator);
		_emit(context, ";\n");
	}
	free(parameters);
	free(declarator);
}

static void _generateFunctionDefinition(CodeGenerationContext * context, FunctionDeclaration * declaration) {
	_emitIndentation(context);
	if (declaration->isStatic) {
		_emit(context, "static ");
	}
	char * parameters = _captureParameters(declaration->parameters);
	char * declarator = parameters != NULL ? _format("%s(%s)", declaration->name, parameters) : NULL;
	if (declarator == NULL) {
		context->failed = true;
	}
	else {
		_generateTypeDeclaration(context, declaration->returnType, declarator);
		_emit(context, " ");
		_generateBlock(context, declaration->body);
		_emit(context, "\n");
	}
	free(parameters);
	free(declarator);
}

static void _generateMain(CodeGenerationContext * context, MainDeclaration * declaration) {
	_emit(context, "int main(int argc, char *argv[]) ");
	_generateBlock(context, declaration->body);
	_emit(context, "\n");
}

static void _generateTypeDeclaration(CodeGenerationContext * context, Type * type, const char * name) {
	Type * baseType = NULL;
	char * declarator = _buildDeclarator(type, name, &baseType);
	const char * baseName = baseType != NULL ? _baseTypeName(baseType) : NULL;
	if (declarator == NULL || baseName == NULL) {
		context->failed = true;
		free(declarator);
		return;
	}
	if (baseType->isConst) {
		_emit(context, "const ");
	}
	_emit(context, "%s", baseName);
	if (baseType->kind == TYPE_STRUCT_KIND
		|| baseType->kind == TYPE_ENUM_KIND
		|| baseType->kind == TYPE_UNION_KIND) {
		_emit(context, " %s", baseType->name);
	}
	if (*declarator != '\0') {
		_emit(context, " %s", declarator);
	}
	free(declarator);
}

static void _generateParameters(CodeGenerationContext * context, ParameterList * parameters) {
	if (parameters == NULL) {
		_emit(context, "void");
		return;
	}
	bool first = true;
	for (ParameterList * node = parameters; node != NULL; node = node->next) {
		if (!first) {
			_emit(context, ", ");
		}
		const char * name = node->parameter->name != NULL ? node->parameter->name : "";
		_generateTypeDeclaration(context, node->parameter->type, name);
		first = false;
	}
}

static void _generateBlock(CodeGenerationContext * context, StatementList * statements) {
	_emit(context, "{\n");
	context->indentation++;
	_generateStatementList(context, statements);
	context->indentation--;
	_emitIndentation(context);
	_emit(context, "}");
}

static void _generateStatementList(CodeGenerationContext * context, StatementList * statements) {
	for (StatementList * node = statements; node != NULL; node = node->next) {
		_generateStatement(context, node->statement);
	}
}

static void _generateStatement(CodeGenerationContext * context, Statement * statement) {
	if (statement == NULL) {
		return;
	}
	switch (statement->kind) {
		case STATEMENT_VARIABLE_DECLARATION:
			_generateVariableDeclaration(context, statement->variableDeclaration, true);
			break;
		case STATEMENT_RETURN:
			_emitIndentation(context);
			_emit(context, "return");
			if (statement->expression != NULL) {
				_emit(context, " ");
				_generateExpression(context, statement->expression);
			}
			_emit(context, ";\n");
			break;
		case STATEMENT_EXPRESSION:
			_emitIndentation(context);
			_generateExpression(context, statement->expression);
			_emit(context, ";\n");
			break;
		case STATEMENT_IF:
			_generateIf(context, statement->ifStatement);
			break;
		case STATEMENT_FOR:
			_generateFor(context, statement->forStatement);
			break;
		case STATEMENT_WHILE:
			_generateWhile(context, statement->whileStatement);
			break;
		case STATEMENT_DO_WHILE:
			_generateDoWhile(context, statement->doWhileStatement);
			break;
		case STATEMENT_SWITCH:
			_generateSwitch(context, statement->switchStatement);
			break;
		case STATEMENT_BREAK:
			_emitLine(context, "break;");
			break;
		case STATEMENT_CONTINUE:
			_emitLine(context, "continue;");
			break;
		case STATEMENT_INLINE_C:
			_generateInlineC(context, statement->inlineC);
			break;
		default:
			break;
	}
}

static void _generateIf(CodeGenerationContext * context, IfStatement * statement) {
	bool first = true;
	for (IfBranch * branch = statement->branches; branch != NULL; branch = branch->next) {
		_emitIndentation(context);
		_emit(context, first ? "if (" : "else if (");
		_generateExpression(context, branch->condition);
		_emit(context, ") ");
		_generateBlock(context, branch->body);
		_emit(context, "\n");
		first = false;
	}
	if (statement->elseBody != NULL) {
		_emitIndentation(context);
		_emit(context, "else ");
		_generateBlock(context, statement->elseBody);
		_emit(context, "\n");
	}
}

static void _generateFor(CodeGenerationContext * context, ForStatement * statement) {
	_emitIndentation(context);
	_emit(context, "for (");
	if (statement->declaresIterator
		&& statement->initializer != NULL
		&& statement->initializer->kind == EXPRESSION_BINARY_OPERATION
		&& statement->initializer->operator == EXPRESSION_OPERATOR_ASSIGN) {
		_emit(context, "int ");
		_generateExpression(context, statement->initializer->left);
		_emit(context, " = ");
		_generateExpression(context, statement->initializer->right);
	}
	else {
		_generateExpression(context, statement->initializer);
	}
	_emit(context, "; ");
	_generateExpression(context, statement->condition);
	_emit(context, "; ");
	_generateExpression(context, statement->update);
	_emit(context, ") ");
	_generateBlock(context, statement->body);
	_emit(context, "\n");
}

static void _generateWhile(CodeGenerationContext * context, WhileStatement * statement) {
	_emitIndentation(context);
	_emit(context, "while (");
	_generateExpression(context, statement->condition);
	_emit(context, ") ");
	_generateBlock(context, statement->body);
	_emit(context, "\n");
}

static void _generateDoWhile(CodeGenerationContext * context, DoWhileStatement * statement) {
	_emitIndentation(context);
	_emit(context, "do ");
	_generateBlock(context, statement->body);
	_emit(context, " while (");
	_generateExpression(context, statement->condition);
	_emit(context, ");\n");
}

static void _generateSwitch(CodeGenerationContext * context, SwitchStatement * statement) {
	_emitIndentation(context);
	_emit(context, "switch (");
	_generateExpression(context, statement->discriminant);
	_emit(context, ") {\n");
	context->indentation++;
	for (SwitchCase * switchCase = statement->cases; switchCase != NULL; switchCase = switchCase->next) {
		_emitIndentation(context);
		if (switchCase->matchExpression == NULL) {
			_emit(context, "default:");
		}
		else {
			_emit(context, "case ");
			_generateExpression(context, switchCase->matchExpression);
			_emit(context, ":");
		}
		_emit(context, " {\n");
		context->indentation++;
		_generateStatementList(context, switchCase->body);
		context->indentation--;
		_emitLine(context, "}");
	}
	context->indentation--;
	_emitLine(context, "}");
}

static void _generateExpression(CodeGenerationContext * context, Expression * expression) {
	if (expression == NULL) {
		return;
	}
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
		case EXPRESSION_FLOAT_LITERAL:
		case EXPRESSION_CHAR_LITERAL:
		case EXPRESSION_STRING_LITERAL:
			_emit(context, "%s", expression->value);
			break;
		case EXPRESSION_INTEGER_LITERAL:
			_generateIntegerLiteral(context, expression->value);
			break;
		case EXPRESSION_NULL_LITERAL:
			_emit(context, "((void *)0)");
			break;
		case EXPRESSION_ARRAY_LITERAL:
			_emit(context, "{");
			_generateExpressionList(context, expression->elements, ", ");
			_emit(context, "}");
			break;
		case EXPRESSION_FUNCTION_CALL:
			_emit(context, "%s(", expression->functionCall->name);
			_generateExpressionList(context, expression->functionCall->arguments, ", ");
			_emit(context, ")");
			break;
		case EXPRESSION_BINARY_OPERATION:
			if (expression->operator == EXPRESSION_OPERATOR_ARRAY_INDEX) {
				_emit(context, "(");
				_generateExpression(context, expression->left);
				_emit(context, ")[");
				_generateExpression(context, expression->right);
				_emit(context, "]");
			}
			else if (expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS
				|| expression->operator == EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS) {
				_emit(context, "(");
				_generateExpression(context, expression->left);
				_emit(context, ")%s%s",
					expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS ? "." : "->",
					expression->right->value);
			}
			else {
				const char * operator = _binaryOperator(expression->operator);
				if (operator == NULL) {
					context->failed = true;
					return;
				}
				_emit(context, "(");
				_generateExpression(context, expression->left);
				_emit(context, " %s ", operator);
				_generateExpression(context, expression->right);
				_emit(context, ")");
			}
			break;
		case EXPRESSION_UNARY_OPERATION:
			if (expression->operator == EXPRESSION_OPERATOR_POSTFIX_INCREMENT
				|| expression->operator == EXPRESSION_OPERATOR_POSTFIX_DECREMENT) {
				_emit(context, "(");
				_generateExpression(context, expression->operand);
				_emit(context,
					expression->operator == EXPRESSION_OPERATOR_POSTFIX_INCREMENT ? "++)" : "--)");
			}
			else {
				const char * operator = _unaryOperator(expression->operator);
				if (operator == NULL) {
					context->failed = true;
					return;
				}
				_emit(context, "(%s", operator);
				_generateExpression(context, expression->operand);
				_emit(context, ")");
			}
			break;
	}
}

static void _generateExpressionList(
	CodeGenerationContext * context,
	ExpressionList * expressions,
	const char * separator) {
	bool first = true;
	for (ExpressionList * node = expressions; node != NULL; node = node->next) {
		if (!first) {
			_emit(context, "%s", separator);
		}
		_generateExpression(context, node->expression);
		first = false;
	}
}

static void _generateIntegerLiteral(CodeGenerationContext * context, const char * value) {
	if (value != NULL
		&& value[0] == '0'
		&& (value[1] == 'o' || value[1] == 'O')) {
		_emit(context, "0%s", value + 2);
	}
	else {
		_emit(context, "%s", value);
	}
}

static bool _isDirective(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE;
}

static bool _isGlobalInlineC(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_INLINE_C;
}

static bool _isEnum(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_ENUM_DECLARATION;
}

static bool _isAggregate(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_AGGREGATE_DECLARATION;
}

static bool _isTypedef(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_TYPEDEF_DECLARATION;
}

static bool _isGlobalVariable(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_VARIABLE_DECLARATION;
}

static bool _isFunctionDefinition(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_FUNCTION_DECLARATION
		&& item->functionDeclaration->body != NULL;
}

static bool _isMain(ProgramItem * item) {
	return item->kind == PROGRAM_ITEM_MAIN_DECLARATION;
}

/** PUBLIC FUNCTIONS */

CompilationStatus executeCodeGeneration(CompilerState * compilerState) {
	logDebugging(_logger, "Executing code generation...");
	if (compilerState == NULL || compilerState->abstractSyntaxtTree == NULL) {
		logError(_logger, "Missing AST for code generation.");
		return FAILED;
	}
	CodeGenerationContext context = {
		.output = stdout,
		.indentation = 0,
		.failed = false
	};
	_generateProgram(&context, (Program *) compilerState->abstractSyntaxtTree);
	if (fflush(context.output) != 0) {
		context.failed = true;
	}
	if (context.failed) {
		logError(_logger, "Failed to generate C output.");
		return FAILED;
	}
	return SUCCEEDED;
}
