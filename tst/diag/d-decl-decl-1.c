typedef void f1(void) { }        /* error */
typedef void f2(a) int a; { }    /* error */
typedef void (*f3)(void) { }    /* error */
typedef void (*f4)(void);
int f5();
int f6(int (*)());
int *();    /* error */
struct { int a; };       /* error */
struct t9 { int a; };
union { int b; };        /* error */
union t12 { int b; };
