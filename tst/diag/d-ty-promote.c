char c; signed char sc; unsigned char uc;
short s; unsigned short us;
enum { X } e;
int a4[sizeof(+c)-sizeof(int)];
int a5[sizeof(+sc)-sizeof(int)];
int a6[sizeof(+uc)-sizeof(int)];
int a7[sizeof(+s)-sizeof(int)];
int a8[sizeof(+us)-sizeof(int)];
