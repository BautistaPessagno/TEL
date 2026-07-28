typedef int Value;

int identity(int Value) {
    return Value;
}

Value preserved;

int local(void) {
    int Value = 3;
    return Value;
}
