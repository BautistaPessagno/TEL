int classify(int n) {
    int total = 0;
    if (n < 0) {
        return -1;
    } else if (n == 0) {
        total = 1;
    } else {
        total = 2;
    }
    for (int i = 0; i < n; i++) {
        if (i == 2) {
            continue;
        }
        total += i;
    }
    while (n > 0) {
        n--;
    }
    do {
        total++;
    } while (total < 3);
    switch (n) {
        case 0: {
            total = 10;
            break;
        }
        case 1: {
            total = 20;
            break;
        }
        default: {
            break;
        }
    }
    return total;
}
