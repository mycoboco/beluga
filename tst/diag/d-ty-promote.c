char c; signed char sc; unsigned char uc;
short s; unsigned short us;
enum { X } e;
int a4[sizeof(+c)-sizeof(int)];     /* error */
int a5[sizeof(+sc)-sizeof(int)];    /* error */
int a6[sizeof(+uc)-sizeof(int)];    /* error */
int a7[sizeof(+s)-sizeof(int)];     /* error */
int a8[sizeof(+us)-sizeof(int)];    /* error */
