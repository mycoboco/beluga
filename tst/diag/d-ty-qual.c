typedef const int ctype;
typedef volatile int vtype;
const ctype x3;       /* error */
const vtype x4;
volatile ctype x5;
volatile vtype x6;    /* error */
const ctype x7[1][1][1];    /* error */
typedef const int carr[1][1][1];
volatile const carr x9;    /* error */
typedef int ftype(void);
ftype const x11;       /* error */
ftype const volatile x11;    /* error */
int * const volatile const x13;    /* error */
