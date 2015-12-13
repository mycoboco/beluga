const int f1(void);
volatile int f2(void);
const volatile int f3(void);
extern const int f4(void);
const int *f5(void);
volatile int (*f6(void))[10];

const int f1(void) {}
volatile int f2(void) {}
const volatile int f3(void) {}
extern const int f4(void) {}
const int *f5(void) {}
volatile int (*f6(void))[10] {}
