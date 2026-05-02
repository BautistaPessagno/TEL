#include "StackADT.h"
#include <stdlib.h>

struct StackADT {
	unsigned int * values;
	size_t capacity;
	size_t size;
};

StackADT * createStackADT(size_t capacity) {
	StackADT * stack = calloc(1, sizeof(StackADT));
	if (stack == NULL) {
		return NULL;
	}
	stack->values = calloc(capacity, sizeof(unsigned int));
	if (stack->values == NULL) {
		free(stack);
		return NULL;
	}
	stack->capacity = capacity;
	return stack;
}

void destroyStackADT(StackADT * stack) {
	if (stack != NULL) {
		if (stack->values != NULL) {
			free(stack->values);
			stack->values = NULL;
		}
		free(stack);
	}
}

bool isEmptyStackADT(StackADT * stack) {
	return stack == NULL || stack->size == 0;
}

bool isFullStackADT(StackADT * stack) {
	return stack == NULL || stack->size == stack->capacity;
}

bool peekStackADT(StackADT * stack, unsigned int * value) {
	if (isEmptyStackADT(stack) || value == NULL) {
		return false;
	}
	*value = stack->values[stack->size - 1];
	return true;
}

bool popStackADT(StackADT * stack, unsigned int * value) {
	if (!peekStackADT(stack, value)) {
		return false;
	}
	stack->size--;
	return true;
}

bool pushStackADT(StackADT * stack, unsigned int value) {
	if (isFullStackADT(stack)) {
		return false;
	}
	stack->values[stack->size] = value;
	stack->size++;
	return true;
}

size_t sizeStackADT(StackADT * stack) {
	return stack == NULL ? 0 : stack->size;
}
