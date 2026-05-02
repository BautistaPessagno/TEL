#ifndef STACK_ADT_HEADER
#define STACK_ADT_HEADER

#include <stdbool.h>
#include <stddef.h>

typedef struct StackADT StackADT;

StackADT * createStackADT(size_t capacity);
void destroyStackADT(StackADT * stack);
bool isEmptyStackADT(StackADT * stack);
bool isFullStackADT(StackADT * stack);
bool peekStackADT(StackADT * stack, unsigned int * value);
bool popStackADT(StackADT * stack, unsigned int * value);
bool pushStackADT(StackADT * stack, unsigned int value);
size_t sizeStackADT(StackADT * stack);

#endif
