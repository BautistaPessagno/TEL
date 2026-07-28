#include "ProgramNormalizer.h"
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <string.h>

enum ProgramCategory {
	PROGRAM_CATEGORY_DIRECTIVE,
	PROGRAM_CATEGORY_INLINE_C,
	PROGRAM_CATEGORY_ENUM,
	PROGRAM_CATEGORY_TYPEDEF,
	PROGRAM_CATEGORY_AGGREGATE,
	PROGRAM_CATEGORY_PROTOTYPE,
	PROGRAM_CATEGORY_GLOBAL,
	PROGRAM_CATEGORY_FUNCTION,
	PROGRAM_CATEGORY_MAIN,
	PROGRAM_CATEGORY_COUNT
};

static void _normalizeType(Type * type);
static void _normalizeExpression(Expression * expression);
static void _normalizeStatementList(StatementList ** statements);

static void _normalizeInteger(char * value) {
	if (value == NULL || value[0] != '0') {
		return;
	}
	if (value[1] == 'o' || value[1] == 'O') {
		value[1] = 'o';
		return;
	}
	if (value[1] == 'x' || value[1] == 'X') {
		return;
	}
	size_t firstNonzero = 0;
	while (value[firstNonzero] == '0' && value[firstNonzero + 1] != '\0') {
		firstNonzero++;
	}
	if (firstNonzero > 0) {
		memmove(value, value + firstNonzero, strlen(value + firstNonzero) + 1);
	}
}

static void _normalizeExpressionList(ExpressionList * expressions) {
	for (ExpressionList * node = expressions; node != NULL; node = node->next) {
		_normalizeExpression(node->expression);
	}
}

static void _normalizeExpression(Expression * expression) {
	if (expression == NULL) {
		return;
	}
	if (expression->kind == EXPRESSION_INTEGER_LITERAL) {
		_normalizeInteger(expression->value);
	}
	if (expression->functionCall != NULL) {
		_normalizeExpressionList(expression->functionCall->arguments);
	}
	_normalizeExpression(expression->operand);
	_normalizeExpression(expression->left);
	_normalizeExpression(expression->right);
	_normalizeExpressionList(expression->elements);
}

static void _normalizeParameterList(ParameterList * parameters) {
	for (ParameterList * node = parameters; node != NULL; node = node->next) {
		if (node->parameter != NULL) {
			_normalizeType(node->parameter->type);
		}
	}
}

static void _normalizeType(Type * type) {
	if (type == NULL) {
		return;
	}
	_normalizeType(type->pointee);
	_normalizeExpression(type->arraySize);
	_normalizeParameterList(type->functionParams);
	_normalizeType(type->returnType);
}

static void _normalizeVariableDeclaration(VariableDeclaration * declaration) {
	if (declaration == NULL) {
		return;
	}
	_normalizeType(declaration->type);
	_normalizeExpression(declaration->initializer);
}

static void _normalizeVariableDeclarationList(VariableDeclarationList * declarations) {
	for (VariableDeclarationList * node = declarations; node != NULL; node = node->next) {
		_normalizeVariableDeclaration(node->declaration);
	}
}

static void _normalizeSwitchCases(SwitchCase * cases) {
	for (SwitchCase * node = cases; node != NULL; node = node->next) {
		_normalizeExpression(node->matchExpression);
		_normalizeStatementList(&node->body);
	}
}

static void _normalizeStatement(Statement * statement) {
	if (statement == NULL) {
		return;
	}
	_normalizeVariableDeclaration(statement->variableDeclaration);
	_normalizeExpression(statement->expression);
	if (statement->ifStatement != NULL) {
		for (IfBranch * branch = statement->ifStatement->branches;
			branch != NULL;
			branch = branch->next) {
			_normalizeExpression(branch->condition);
			_normalizeStatementList(&branch->body);
		}
		_normalizeStatementList(&statement->ifStatement->elseBody);
	}
	if (statement->forStatement != NULL) {
		_normalizeExpression(statement->forStatement->initializer);
		_normalizeExpression(statement->forStatement->condition);
		_normalizeExpression(statement->forStatement->update);
		_normalizeStatementList(&statement->forStatement->body);
	}
	if (statement->whileStatement != NULL) {
		_normalizeExpression(statement->whileStatement->condition);
		_normalizeStatementList(&statement->whileStatement->body);
	}
	if (statement->doWhileStatement != NULL) {
		_normalizeStatementList(&statement->doWhileStatement->body);
		_normalizeExpression(statement->doWhileStatement->condition);
	}
	if (statement->switchStatement != NULL) {
		_normalizeExpression(statement->switchStatement->discriminant);
		_normalizeSwitchCases(statement->switchStatement->cases);
	}
}

static void _normalizeStatementList(StatementList ** statements) {
	StatementList ** link = statements;
	while (*link != NULL) {
		StatementList * node = *link;
		if (node->statement == NULL || node->statement->kind == STATEMENT_EMPTY) {
			*link = node->next;
			node->next = NULL;
			destroyStatementList(node);
			continue;
		}
		_normalizeStatement(node->statement);
		link = &node->next;
	}
}

static void _normalizeProgramItem(ProgramItem * item) {
	if (item == NULL) {
		return;
	}
	switch (item->kind) {
		case PROGRAM_ITEM_VARIABLE_DECLARATION:
			_normalizeVariableDeclaration(item->variableDeclaration);
			break;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			if (item->functionDeclaration != NULL) {
				_normalizeParameterList(item->functionDeclaration->parameters);
				_normalizeType(item->functionDeclaration->returnType);
				_normalizeStatementList(&item->functionDeclaration->body);
			}
			break;
		case PROGRAM_ITEM_MAIN_DECLARATION:
			if (item->mainDeclaration != NULL) {
				_normalizeStatementList(&item->mainDeclaration->body);
			}
			break;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION:
			if (item->aggregateDeclaration != NULL) {
				_normalizeVariableDeclarationList(item->aggregateDeclaration->fields);
			}
			break;
		case PROGRAM_ITEM_ENUM_DECLARATION:
			if (item->enumDeclaration != NULL) {
				for (EnumMemberList * node = item->enumDeclaration->members;
					node != NULL;
					node = node->next) {
					if (node->member != NULL) {
						_normalizeInteger(node->member->value);
					}
				}
			}
			break;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION:
			if (item->typedefDeclaration != NULL) {
				_normalizeType(item->typedefDeclaration->type);
			}
			break;
		default:
			break;
	}
}

static bool _hasFunctionDefinition(ProgramItemList * items, const char * name) {
	for (ProgramItemList * node = items; node != NULL; node = node->next) {
		ProgramItem * item = node->item;
		if (item != NULL
			&& item->kind == PROGRAM_ITEM_FUNCTION_DECLARATION
			&& item->functionDeclaration != NULL
			&& item->functionDeclaration->body != NULL
			&& strcmp(item->functionDeclaration->name, name) == 0) {
			return true;
		}
	}
	return false;
}

static bool _hasEarlierPrototype(ProgramItemList * items, ProgramItemList * current) {
	const char * name = current->item->functionDeclaration->name;
	for (ProgramItemList * node = items; node != current; node = node->next) {
		ProgramItem * item = node->item;
		if (item != NULL
			&& item->kind == PROGRAM_ITEM_FUNCTION_DECLARATION
			&& item->functionDeclaration != NULL
			&& item->functionDeclaration->body == NULL
			&& strcmp(item->functionDeclaration->name, name) == 0) {
			return true;
		}
	}
	return false;
}

static void _removeEmptyAndRedundantItems(Program * program) {
	ProgramItemList ** link = &program->items;
	while (*link != NULL) {
		ProgramItemList * node = *link;
		ProgramItem * item = node->item;
		bool remove = item == NULL || item->kind == PROGRAM_ITEM_EMPTY;
		if (!remove
			&& item->kind == PROGRAM_ITEM_FUNCTION_DECLARATION
			&& item->functionDeclaration != NULL
			&& item->functionDeclaration->body == NULL) {
			remove = _hasFunctionDefinition(program->items, item->functionDeclaration->name)
				|| _hasEarlierPrototype(program->items, node);
		}
		if (remove) {
			*link = node->next;
			node->next = NULL;
			destroyProgramItemList(node);
			continue;
		}
		_normalizeProgramItem(item);
		link = &node->next;
	}
}

static enum ProgramCategory _programCategory(ProgramItem * item) {
	switch (item->kind) {
		case PROGRAM_ITEM_PREPROCESSOR_DIRECTIVE: return PROGRAM_CATEGORY_DIRECTIVE;
		case PROGRAM_ITEM_INLINE_C: return PROGRAM_CATEGORY_INLINE_C;
		case PROGRAM_ITEM_ENUM_DECLARATION: return PROGRAM_CATEGORY_ENUM;
		case PROGRAM_ITEM_TYPEDEF_DECLARATION: return PROGRAM_CATEGORY_TYPEDEF;
		case PROGRAM_ITEM_AGGREGATE_DECLARATION: return PROGRAM_CATEGORY_AGGREGATE;
		case PROGRAM_ITEM_VARIABLE_DECLARATION: return PROGRAM_CATEGORY_GLOBAL;
		case PROGRAM_ITEM_MAIN_DECLARATION: return PROGRAM_CATEGORY_MAIN;
		case PROGRAM_ITEM_FUNCTION_DECLARATION:
			return item->functionDeclaration->body == NULL
				? PROGRAM_CATEGORY_PROTOTYPE
				: PROGRAM_CATEGORY_FUNCTION;
		default:
			return PROGRAM_CATEGORY_COUNT;
	}
}

static void _stablyOrderItems(Program * program) {
	ProgramItemList * heads[PROGRAM_CATEGORY_COUNT] = {NULL};
	ProgramItemList * tails[PROGRAM_CATEGORY_COUNT] = {NULL};
	ProgramItemList * node = program->items;
	while (node != NULL) {
		ProgramItemList * next = node->next;
		node->next = NULL;
		enum ProgramCategory category = _programCategory(node->item);
		if (category < PROGRAM_CATEGORY_COUNT) {
			if (heads[category] == NULL) {
				heads[category] = node;
			}
			else {
				tails[category]->next = node;
			}
			tails[category] = node;
		}
		node = next;
	}
	program->items = NULL;
	ProgramItemList ** tail = &program->items;
	for (enum ProgramCategory category = PROGRAM_CATEGORY_DIRECTIVE;
		category < PROGRAM_CATEGORY_COUNT;
		category++) {
		if (heads[category] != NULL) {
			*tail = heads[category];
			tail = &tails[category]->next;
		}
	}
}

CompilationStatus normalizeProgram(CompilerState * compilerState) {
	if (compilerState == NULL || compilerState->abstractSyntaxtTree == NULL) {
		return FAILED;
	}
	Program * program = compilerState->abstractSyntaxtTree;
	_removeEmptyAndRedundantItems(program);
	_stablyOrderItems(program);
	return SUCCEEDED;
}
