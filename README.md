# TinyC
A just-in-time compiler for c minus
***

#### Features:
+ One-pass compiler
+ Hashtable based lexical analysis
+ Top-down operator precedence parsing, LL(k)
+ JIT on x86
+ Interactive console
+ Windows/Linux/MacOSX support
+ No more than 1000 lines of code

***
#### Example:   
        scan@ubuntu:~/TinyC$ make
        g++ -m32 -g pch.h
        g++ -MMD -m32     -g   -c -o main.o main.cpp
        g++ -MMD -m32     -g   -c -o pch.o pch.cpp
        g++  -o main main.o pch.o -MMD -m32     -g -ldl
        scan@ubuntu:~/TinyC$ ./main
        >>> 1+2*(3-5)
        -3
        >>> loadFile("test/test.c")
        0
        >>> factorial(10)
        3628800
        >>> feb2(11)
        89
        >>> feb2(10)+feb2(9)
        89
        >>> int pow(int x, int y) { int r = 1; for (int i = 0; i < y; ++i) r = r * x; return r; }
        >>> pow(2, 10)
        1024
        >>> 32 * pow(2, 10)
        32768
        >>> printPrime(10)
        ========== printPrime ==========
        3
        5
        7
        
        1
        >>> int loop(int n) { int start = clock(); for (int i = 0; i < n; ++i); printf("clocks:%d\n", clock() - start); return 0;}
        >>> loop(1000)
        clocks:0
        0
        >>> loop(1000*1000)
        clocks:0
        0
        >>> loop(pow(1000, 3))
        clocks:4360000
        0
        >>>
