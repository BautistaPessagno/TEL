#include <stdio.h>
#define ANSWER 42

enum Color {
    RED,
    GREEN = 2,
    BLUE
};

struct Point {
    int x;
    int y;
};

union Number {
    int integer;
    double decimal;
};

typedef unsigned int Count;
typedef int (*Comparator)(int left, int right);

struct Point origin;
Count count;
