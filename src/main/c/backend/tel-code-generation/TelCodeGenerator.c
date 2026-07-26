#include "TelCodeGenerator.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	FILE * output;
	unsigned int indentation;
	bool failed;
} TelGenerationContext;

static void _generateType(TelGenerationContext * context, Type * type);
static void _generateExpression(TelGenerationContext * context, Expression * expression);
static void _generateStatementList(
	TelGenerationContext * context,
	StatementList * statements,
	StatementList * excluded);

static void _emit(TelGenerationContext * context, const char * format, ...) {
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

static void _emitIndentation(TelGenerationContext * context) {
	for (unsigned int index = 0; index < context->indentation; index++) {
		_emit(context, "    ");
	}
}

static void _emitLine(TelGenerationContext * context, const char * format, ...) {
	_emitIndentation(context);
	va_list arguments;
	va_start(arguments, format);
	if (!context->failed && vfprintf(context->output, format, arguments) < 0) {
		context->failed = true;
	}
	va_end(arguments);
	_emit(context, "\n");
}

static const char * _baseTypeName(Type * type) {
	switch (type->kind) {
		case TYPE_INT_KIND: return "int";
		case TYPE_CHAR_KIND: return "char";
		case TYPE_FLOAT_KIND: return "float";
		case TYPE_DOUBLE_KIND: return "double";
		case TYPE_VOID_KIND: return "void";
		case TYPE_UINT_KIND: return "uint";
		case TYPE_ULI_KIND: return "uli";
		case TYPE_LONG_KIND: return "long";
		case TYPE_SHORT_KIND: return "short";
		default: return NULL;
	}
}

static void _generateBareParameterTypes(
	TelGenerationContext * context,
	ParameterList * parameters) {
	for (ParameterList * node = parameters; node != NULL; node = node->next) {
		_emit(context, " ");
		_generateType(context, node->parameter->type);
	}
}

static void _generateType(TelGenerationContext * context, Type * type) {
	if (type == NULL) {
		context->failed = true;
		return;
	}
	if (type->kind == TYPE_POINTER_KIND) {
		_generateType(context, type->pointee);
		_emit(context, "*");
		return;
	}
	if (type->kind == TYPE_ARRAY_KIND) {
		_generateType(context, type->pointee);
		_emit(context, "[");
		_generateExpression(context, type->arraySize);
		_emit(context, "]");
		return;
	}
	if (type->kind == TYPE_FUNCTION_POINTER_KIND) {
		_emit(context, "(fn*");
		_generateBareParameterTypes(context, type->functionParams);
		if (type->returnType != NULL && type->returnType->kind != TYPE_VOID_KIND) {
			_emit(context, " -> ");
			_generateType(context, type->returnType);
		}
		_emit(context, ")");
		return;
	}
	if (type->isConst) {
		_emit(context, "const ");
	}
	const char * baseName = _baseTypeName(type);
	if (baseName != NULL) {
		_emit(context, "%s", baseName);
		return;
	}
	switch (type->kind) {
		case TYPE_NAMED_KIND:
			_emit(context, "%s", type->name);
			break;
		case TYPE_STRUCT_KIND:
			_emit(context, "struct %s", type->name);
			break;
		case TYPE_ENUM_KIND:
			_emit(context, "enum %s", type->name);
			break;
		case TYPE_UNION_KIND:
			_emit(context, "union %s", type->name);
			break;
		default:
			context->failed = true;
			break;
	}
}

static const char * _operatorText(ExpressionOperator operator) {
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
		case EXPRESSION_OPERATOR_UNARY_PLUS: return "+";
		case EXPRESSION_OPERATOR_UNARY_MINUS: return "-";
		case EXPRESSION_OPERATOR_LOGICAL_NOT: return "!";
		case EXPRESSION_OPERATOR_BITWISE_NOT: return "~";
		case EXPRESSION_OPERATOR_DEREFERENCE: return "*";
		case EXPRESSION_OPERATOR_ADDRESS_OF: return "&";
		case EXPRESSION_OPERATOR_PREFIX_INCREMENT: return "++";
		case EXPRESSION_OPERATOR_PREFIX_DECREMENT: return "--";
		case EXPRESSION_OPERATOR_POSTFIX_INCREMENT: return "++";
		case EXPRESSION_OPERATOR_POSTFIX_DECREMENT: return "--";
		default: return NULL;
	}
}

static int _expressionPrecedence(Expression * expression) {
	if (expression == NULL) {
		return 100;
	}
	if (expression->kind != EXPRESSION_BINARY_OPERATION
		&& expression->kind != EXPRESSION_UNARY_OPERATION) {
		return 100;
	}
	switch (expression->operator) {
		case EXPRESSION_OPERATOR_ASSIGN:
		case EXPRESSION_OPERATOR_ADD_ASSIGN:
		case EXPRESSION_OPERATOR_SUBTRACT_ASSIGN:
		case EXPRESSION_OPERATOR_MULTIPLY_ASSIGN:
		case EXPRESSION_OPERATOR_DIVIDE_ASSIGN:
		case EXPRESSION_OPERATOR_MODULO_ASSIGN:
		case EXPRESSION_OPERATOR_BITWISE_AND_ASSIGN:
		case EXPRESSION_OPERATOR_BITWISE_OR_ASSIGN:
		case EXPRESSION_OPERATOR_BITWISE_XOR_ASSIGN:
		case EXPRESSION_OPERATOR_SHIFT_LEFT_ASSIGN:
		case EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN:
			return 1;
		case EXPRESSION_OPERATOR_LOGICAL_OR: return 2;
		case EXPRESSION_OPERATOR_LOGICAL_AND: return 3;
		case EXPRESSION_OPERATOR_BITWISE_OR: return 4;
		case EXPRESSION_OPERATOR_BITWISE_XOR: return 5;
		case EXPRESSION_OPERATOR_BITWISE_AND: return 6;
		case EXPRESSION_OPERATOR_EQUAL:
		case EXPRESSION_OPERATOR_NOT_EQUAL:
			return 7;
		case EXPRESSION_OPERATOR_LESS_THAN:
		case EXPRESSION_OPERATOR_GREATER_THAN:
		case EXPRESSION_OPERATOR_LESS_EQUAL:
		case EXPRESSION_OPERATOR_GREATER_EQUAL:
			return 8;
		case EXPRESSION_OPERATOR_SHIFT_LEFT:
		case EXPRESSION_OPERATOR_SHIFT_RIGHT:
			return 9;
		case EXPRESSION_OPERATOR_ADD:
		case EXPRESSION_OPERATOR_SUBTRACT:
			return 10;
		case EXPRESSION_OPERATOR_MULTIPLY:
		case EXPRESSION_OPERATOR_DIVIDE:
		case EXPRESSION_OPERATOR_MODULO:
			return 11;
		case EXPRESSION_OPERATOR_UNARY_PLUS:
		case EXPRESSION_OPERATOR_UNARY_MINUS:
		case EXPRESSION_OPERATOR_LOGICAL_NOT:
		case EXPRESSION_OPERATOR_BITWISE_NOT:
		case EXPRESSION_OPERATOR_DEREFERENCE:
		case EXPRESSION_OPERATOR_ADDRESS_OF:
		case EXPRESSION_OPERATOR_PREFIX_INCREMENT:
		case EXPRESSION_OPERATOR_PREFIX_DECREMENT:
			return 12;
		case EXPRESSION_OPERATOR_ARRAY_INDEX:
		case EXPRESSION_OPERATOR_MEMBER_ACCESS:
		case EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS:
		case EXPRESSION_OPERATOR_POSTFIX_INCREMENT:
		case EXPRESSION_OPERATOR_POSTFIX_DECREMENT:
			return 13;
	}
	return 100;
}

static bool _isAssignmentOperator(ExpressionOperator operator) {
	return operator >= EXPRESSION_OPERATOR_ASSIGN
		&& operator <= EXPRESSION_OPERATOR_SHIFT_RIGHT_ASSIGN;
}

static void _generateChildExpression(
	TelGenerationContext * context,
	Expression * child,
	int parentPrecedence,
	bool isRightChild,
	bool parentIsAssignment) {
	int childPrecedence = _expressionPrecedence(child);
	bool needsParentheses = childPrecedence < parentPrecedence
		|| (childPrecedence == parentPrecedence
			&& ((parentIsAssignment && !isRightChild)
				|| (!parentIsAssignment && isRightChild)));
	if (needsParentheses) {
		_emit(context, "(");
	}
	_generateExpression(context, child);
	if (needsParentheses) {
		_emit(context, ")");
	}
}

static void _generateExpressionList(
	TelGenerationContext * context,
	ExpressionList * expressions,
	const char * separator,
	bool parenthesizeComposite) {
	bool first = true;
	for (ExpressionList * node = expressions; node != NULL; node = node->next) {
		if (!first) {
			_emit(context, "%s", separator);
		}
		bool parentheses = parenthesizeComposite
			&& _expressionPrecedence(node->expression) < 100;
		if (parentheses) {
			_emit(context, "(");
		}
		_generateExpression(context, node->expression);
		if (parentheses) {
			_emit(context, ")");
		}
		first = false;
	}
}

static void _generateExpression(TelGenerationContext * context, Expression * expression) {
	if (expression == NULL) {
		return;
	}
	switch (expression->kind) {
		case EXPRESSION_IDENTIFIER:
		case EXPRESSION_INTEGER_LITERAL:
		case EXPRESSION_FLOAT_LITERAL:
		case EXPRESSION_CHAR_LITERAL:
		case EXPRESSION_STRING_LITERAL:
			_emit(context, "%s", expression->value);
			break;
		case EXPRESSION_NULL_LITERAL:
			_emit(context, "null");
			break;
		case EXPRESSION_ARRAY_LITERAL:
			_emit(context, "{");
			_generateExpressionList(context, expression->elements, " ", true);
			_emit(context, "}");
			break;
		case EXPRESSION_FUNCTION_CALL:
			_emit(context, "%s(", expression->functionCall->name);
			_generateExpressionList(context, expression->functionCall->arguments, ", ", false);
			_emit(context, ")");
			break;
		case EXPRESSION_UNARY_OPERATION: {
			const char * operator = _operatorText(expression->operator);
			if (operator == NULL) {
				context->failed = true;
				break;
			}
			bool postfix = expression->operator == EXPRESSION_OPERATOR_POSTFIX_INCREMENT
				|| expression->operator == EXPRESSION_OPERATOR_POSTFIX_DECREMENT;
			if (!postfix) {
				_emit(context, "%s", operator);
			}
			bool parentheses = _expressionPrecedence(expression->operand)
				<= _expressionPrecedence(expression);
			if (parentheses) {
				_emit(context, "(");
			}
			_generateExpression(context, expression->operand);
			if (parentheses) {
				_emit(context, ")");
			}
			if (postfix) {
				_emit(context, "%s", operator);
			}
			break;
		}
		case EXPRESSION_BINARY_OPERATION:
			if (expression->operator == EXPRESSION_OPERATOR_ARRAY_INDEX) {
				_generateChildExpression(
					context,
					expression->left,
					_expressionPrecedence(expression),
					false,
					false);
				_emit(context, "[");
				_generateExpression(context, expression->right);
				_emit(context, "]");
			}
			else if (expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS
				|| expression->operator == EXPRESSION_OPERATOR_POINTER_MEMBER_ACCESS) {
				_generateChildExpression(
					context,
					expression->left,
					_expressionPrecedence(expression),
					false,
					false);
				_emit(context,
					expression->operator == EXPRESSION_OPERATOR_MEMBER_ACCESS ? "." : "->");
				_generateExpression(context, expression->right);
			}
			else {
				const char * operator = _operatorText(expression->operator);
				if (operator == NULL) {
					context->failed = true;
					break;
				}
				int precedence = _expressionPrecedence(expression);
				bool assignment = _isAssignmentOperator(expression->operator);
				_generateChildExpression(context, expression->left, precedence, false, assignment);
				_emit(context, " %s ", operator);
				_generateChildExpression(context, expression->right, precedence, true, assignment);
			}
			break;
	}
}

static void _generateVariableDeclaration(
	TelGenerationContext * context,
	VariableDeclaration * declaration) {
	_emitIndentation(context);
	if (declaration->isStatic) {
		_emit(context, "stc ");
	}
	_emit(context, "%s:", declaration->name);
	_generateType(context, declaration->type);
	if (declaration->initializer != NULL) {
		_emit(context, " = ");
		_generateExpression(context, declaration->initializer);
	}
	_emit(context, "\n");
}

static void _generateInlineC(TelGenerationContext * context, const char * code) {
	_emitLine(context, "`");
	if (code != NULL && *code != '\0') {
		_emit(context, "%s", code);
		if (code[strlen(code) - 1] != '\n') {
			_emit(context, "\n");
		}
	}
	_emitLine(context, "`");
}

static StatementList * _lastStatementNode(StatementList * statements) {
	StatementList * last = NULL;
	for (StatementList * node = statements; node != NULL; node = node->next) {
		last = node;
	}
	return last;
}

static void _generateIf(TelGenerationContext * context, IfStatement * statement) {
	bool first = true;
	for (IfBranch * branch = statement->branches; branch != NULL; branch = branch->next) {
		_emitIndentation(context);
		_emit(context, first ? "if " : "elif ");
		_generateExpression(context, branch->condition);
		_emit(context, "\n");
		context->indentation++;
		_generateStatementList(context, branch->body, NULL);
		context->indentation--;
		first = false;
	}
	if (statement->elseBody != NULL) {
		_emitLine(context, "else");
		context->indentation++;
		_generateStatementList(context, statement->elseBody, NULL);
		context->indentation--;
	}
}

static void _generateFordBound(TelGenerationContext * context, Expression * expression) {
	bool atomic = expression != NULL
		&& (expression->kind == EXPRESSION_IDENTIFIER
			|| expression->kind == EXPRESSION_INTEGER_LITERAL);
	if (!atomic) {
		_emit(context, "(");
	}
	_generateExpression(context, expression);
	if (!atomic) {
		_emit(context, ")");
	}
}

static void _generateFor(TelGenerationContext * context, ForStatement * statement) {
	_emitIndentation(context);
	if (statement->declaresIterator
		&& statement->initializer != NULL
		&& statement->initializer->kind == EXPRESSION_BINARY_OPERATION
		&& statement->initializer->operator == EXPRESSION_OPERATOR_ASSIGN) {
		_emit(context, "ford %s ",
			statement->initializer->left->value);
		_generateFordBound(context, statement->initializer->right);
		_emit(context, " ");
		_generateFordBound(context, statement->condition->right);
	}
	else {
		_emit(context, "for ");
		_generateExpression(context, statement->initializer);
		_emit(context, ", ");
		_generateExpression(context, statement->condition);
		_emit(context, ", ");
		_generateExpression(context, statement->update);
	}
	_emit(context, "\n");
	context->indentation++;
	_generateStatementList(context, statement->body, NULL);
	context->indentation--;
}

static void _generateSwitch(TelGenerationContext * context, SwitchStatement * statement) {
	_emitIndentation(context);
	_emit(context, "switch ");
	_generateExpression(context, statement->discriminant);
	_emit(context, "\n");
	context->indentation++;
	for (SwitchCase * switchCase = statement->cases;
		switchCase != NULL;
		switchCase = switchCase->next) {
		StatementList * last = _lastStatementNode(switchCase->body);
		bool trailingBreak = last != NULL
			&& last->statement != NULL
			&& last->statement->kind == STATEMENT_BREAK;
		bool onlyBreak = trailingBreak && last == switchCase->body;
		_emitIndentation(context);
		if (switchCase->matchExpression == NULL) {
			_emit(context, "default");
		}
		else {
			_generateExpression(context, switchCase->matchExpression);
		}
		if (onlyBreak && switchCase->matchExpression == NULL) {
			_emit(context, " break\n");
			continue;
		}
		_emit(context, trailingBreak && !onlyBreak ? " ->\n" : ":\n");
		context->indentation++;
		_generateStatementList(context, switchCase->body, trailingBreak && !onlyBreak ? last : NULL);
		context->indentation--;
	}
	context->indentation--;
}

static void _generateStatement(TelGenerationContext * context, Statement * statement) {
	if (statement == NULL) {
		return;
	}
	switch (statement->kind) {
		case STATEMENT_VARIABLE_DECLARATION:
			_generateVariableDeclaration(context, statement->variableDeclaration);
			break;
		case STATEMENT_RETURN:
			_emitIndentation(context);
			_emit(context, "ret");
			if (statement->expression != NULL) {
				_emit(context, " ");
				_generateExpression(context, statement->expression);
			}
			_emit(context, "\n");
			break;
		case STATEMENT_EXPRESSION:
			_emitIndentation(context);
			_generateExpression(context, statement->expression);
			_emit(context, "\n");
			break;
		case STATEMENT_IF:
			_generateIf(context, statement->ifStatement);
			break;
		case STATEMENT_FOR:
			_generateFor(context, statement->forStatement);
			break;
		case STATEMENT_WHILE:
			_emitIndentation(context);
			_emit(context, "while ");
			_generateExpression(context, statement->whileStatement->condition);
			_emit(context, "\n");
			context->indentation++;
			_generateStatementList(context, statement->whileStatement->body, NULL);
			context->indentation--;
			break;
		case STATEMENT_DO_WHILE:
			_emitLine(context, "dw");
			context->indentation++;
			_generateStatementList(context, statement->doWhileStatement->body, NULL);
			context->indentation--;
			_emitIndentation(context);
			_emit(context, "(");
			_generateExpression(context, statement->doWhileStatement->condition);
			_emit(context, ")\n");
			break;
		case STATEMENT_SWITCH:
			_generateSwitch(context, statement->switchStatement);
			break;
		case STATEMENT_BREAK:
			_emitLine(context, "break");
			break;
		case STATEMENT_CONTINUE:
			_emitLine(context, "cnt");
			break;
		case STATEMENT_INLINE_C:
			_generateInlineC(context, statement->inlineC);
			break;
		default:
			break;
	}
}

static void _generateStatementList(
	TelGenerationContext * context,
	StatementList * statements,
	StatementList * excluded) {
	for (StatementList * node = statements; node != NULL; node = node->next) {
		if (node != excluded) {
			_generateStatement(context, node->statement);
		}
	}
}

static void _generateParameters(
	TelGenerationContext * context,
	ParameterList * parameters) {
	for (ParameterList * node = parameters; node != NULL; node = node->next) {
		if (node->parameter->name == NULL) {
			context->failed = true;
			return;
		}
		_emit(context, " %s:", node->parameter->name);
		_generateType(context, node->parameter->type);
	}
}

static void _generateFunction(
	TelGenerationContext * context,
	FunctionDeclaration * declaration) {
	if (declaration->isStatic) {
		_emit(context, "stc ");
	}
	_emit(context, "fn %s", declaration->name);
	_generateParameters(context, declaration->parameters);
	if (declaration->returnType->kind != TYPE_VOID_KIND) {
		_emit(context, " -> ");
		_generateType(context, declaration->returnType);
	}
	if (declaration->body == NULL) {
		_emit(context, ";\n");
		return;
	}
	_emit(context, "\n");
	context->indentation++;
	_generateStatementList(context, declaration->body, NULL);
	context->indentation--;
}

static void _generateProgramItem(TelGenerationContext * context, ProgramItem * item) {
	switch (item->kind) {
		case PROGRAM_ITEM_VARIABLE_DECLARATION:
			_generateVariableDeclaration(context, item->variableDeclaration);
			break;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			_generateFunction(context, item->functionDeclaration);
			break;
		case PROGRAM_ITEM_MAIN_DECLARATION:
			_emit(context, "main\n");
			context->indentation++;
			_generateStatementList(context, item->mainDeclaration->body, NULL);
			context->indentation--;
			break;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION:
			_emit(context,
				"%s %s\n",
				item->aggregateDeclaration->kind == AGGREGATE_STRUCT_KIND ? "struct" : "union",
				item->aggregateDeclaration->name);
			context->indentation++;
			for (VariableDeclarationList * node = item->aggregateDeclaration->fields;
				node != NULL;
				node = node->next) {
				_generateVariableDeclaration(context, node->declaration);
			}
			context->indentation--;
			break;
		case PROGRAM_ITEM_ENUM_DECLARATION:
			_emit(context, "enum %s\n", item->enumDeclaration->name);
			context->indentation++;
			for (EnumMemberList * node = item->enumDeclaration->members;
				node != NULL;
				node = node->next) {
				_emitIndentation(context);
				_emit(context, "%s", node->member->name);
				if (node->member->value != NULL) {
					_emit(context, " = %s", node->member->value);
				}
				_emit(context, "\n");
			}
			context->indentation--;
			break;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION:
			_emit(context, "typedef %s:", item->typedefDeclaration->name);
			_generateType(context, item->typedefDeclaration->type);
			_emit(context, "\n");
			break;
		case PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE:
			_emit(context,
				item->preprocessorDirective->kind == PREPROCESSOR_INCLUDE_DIRECTIVE
					? "#include %s\n"
					: "#define %s\n",
				item->preprocessorDirective->value);
			break;
		case PROGRAM_ITEM_INLINE_C:
			_generateInlineC(context, item->inlineC);
			break;
		default:
			break;
	}
}

CompilationStatus executeTelCodeGeneration(CompilerState * compilerState, FILE * output) {
	if (compilerState == NULL || compilerState->abstractSyntaxtTree == NULL || output == NULL) {
		return FAILED;
	}
	TelGenerationContext context = {
		.output = output,
		.indentation = 0,
		.failed = false
	};
	Program * program = compilerState->abstractSyntaxtTree;
	bool first = true;
	for (ProgramItemList * node = program->items; node != NULL; node = node->next) {
		if (!first) {
			_emit(&context, "\n");
		}
		_generateProgramItem(&context, node->item);
		first = false;
	}
	if (fflush(output) != 0) {
		context.failed = true;
	}
	return context.failed ? FAILED : SUCCEEDED;
}
