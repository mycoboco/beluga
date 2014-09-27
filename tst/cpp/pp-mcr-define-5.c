#define FOO1(a) test
#define FOO1    test

#define FOO2(a) test
#define FOO2 no test

#define FOO3    test
#define FOO3(a) test

#define FOO4    test
#define FOO4(a) test a

#define FOO5(a) a
#define FOO5(b) b

#define FOO6(a) a
#define FOO6(b) a

#define FOO7()  test
#define FOO7(a) test

#define FOO8(a)    test
#define FOO8(a, b) test

#define FOO9(a, b) test
#define FOO9(a)    test
