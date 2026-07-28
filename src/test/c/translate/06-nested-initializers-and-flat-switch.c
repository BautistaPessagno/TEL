int matrix[2][2] = {{1, 2}, {3, 4}};

int main(void) {
    switch (matrix[0][0]) {
        case 0:
            return 1;
        default:
            return 2;
    }
}
