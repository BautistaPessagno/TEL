typedef int Value;

int copy_value(void) {
    int Value = 1, copy = Value;
    return copy;
}

int self_value(void) {
    int Value = Value;
    return Value;
}
