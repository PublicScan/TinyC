int echo(string str, int b) {
    printf("echo %s\n", str);
    return b;
}
void testAnd() {
    printf("========== testAnd 1 ==========\n");
    echo("1", true) && echo("1", true);
    printf("========== testAnd 2 ==========\n");
    echo("0", false) && echo("1", true);
}
void testOr() {
    printf("========== testOr 1 ==========\n");
    echo("0", false) || echo("0", false);
    printf("========== testOr 1 ==========\n");
    echo("1", true) || echo("0", false);
}
void printPrime(int n) {
    printf("========== printPrime ==========\n");
    for (int i = 3; i < n; ++i) {
        int flag = true;
        for (int j = 2; j <= i / 2; ++j) {
            if (i % j == 0) {
                flag = false;
                break;
            }
        }
        if (flag) printf("%d\n", i) ;
    }
    printf("\n");
}
int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}
void printFeb1() {
    printf("========== printFeb1 ==========\n");
    int i = 1, j = 1;
    while (i < 100) {
        printf("%d\t", j);
        int t = i + j;
        j = i;
        i = t;
    }
    printf("\n");
}
int feb2(int n) {
    // it's hard to print function name, so...
    if (n <= 2) { return 1; }
    return feb2(n - 1) + feb2(n - 2);
}
void printFeb2() {
    printf("========== printFeb2 ==========\n");
    int i = 1;
    while (i <= 10) {
        printf("%d\t", feb2(i)) ;
        i = i + 1;
    }
    printf("\n");
}
void print9x9() {
    printf("========== print9x9 ==========\n");
    for (int i = 1; i <= 9; ++i) {
        for (int j = 1; j <= i; ++j) {
            printf("%dx%d=%-2d ", j, i, i * j);
        }
        printf("\n");
    }
}
void perform() {
    printf("========== perform ==========\n");
    int start = clock();
    for (int i = 0; i < 1000000; ++i);
    printf("loop 1000000 times: %d\n", clock() - start);
}
int main()  {
    // test for comment
    testAnd(); // test for comment too
    testOr();
    printPrime(30);
    printf("factorial %d,%d\n", 10, factorial(10));
    printFeb1();
    printFeb2();
    print9x9();
    perform();
    printf("15-3*(2*2+(7-2)) = %d\n", 15-3*(2*2+(7-2)));
    return 0;
}
