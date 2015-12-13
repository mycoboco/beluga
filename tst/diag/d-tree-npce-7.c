/* -Wv --std=c90 */

int a1[(int)(double)3];         /* no ice */
int a2[1 || (int)(double)3];    /* no ice */
int x1 = (double)3;

int x2 = (int *)(void *)0 - (int *)(void *)0;            /* no ace */
int x3 = (int *)0 - (int *)0;                            /* no ace */
int x4 = 1 || (int *)(void *)0 - (int *)(void *)0;       /* no ace */
int x5 = 1 || (int *)0 - (int *)0;                       /* no ace */
int x6 = (1)? 0: (int *)(void *)0 - (int *)(void *)0;    /* no ace */
int x7 = (1)? 0: (int *)0 - (int *)0;                    /* no ace */
int x8 = 0 && ((int *)(void *)0-(int *)(void *)0, 1);    /* no ace */
int x9 = 0 && ((int *)0-(int *)0, 1);                    /* no ace */

int x10 = (int)(void *)0;              /* no ace */
int x11 = 0 && (int)(void *)0;         /* no ace */
int x12 = (int)&x10;                   /* no ace */
int x13 = 0 && (int)&x10;              /* no ace */
int x14 = 0 && ((int)(void *)0, 1);    /* no ace */
int x15 = 0 && ((int)&x10, 1);         /* no ace */

int *x16 = (int *)(int)(void *)0;                         /* no addr */
int *x17 = (1)? 0: (int *)(int)(void *)0;
int *x18 = (1)? 0: ((int *)(int)(void *)0, (void *)0);
int *x19 = (1)? (int *)(int)(void *)0: 0;                 /* no addr */
int *x20 = (int *)(int)&x16;                              /* no addr */
int *x21 = (1)? 0: (int *)(int)&x16;
int *x22 = (1)? 0: ((int *)(int)&x16, (void *)0);
int *x23 = (1)? (int *)(int)&x16: 0;                      /* no addr */

int *x24 = (int *)0;
int *x25 = (int *)1;

int x26 = (double)3 + 3.14;
int x27 = (1, 3.14, 3.14);     /* no ace */

int x28 = (0)? 0: (int *)(void *)0 - (int *)(void *)0;       /* no ace */
int x29 = 1 && (int *)(void *)0 - (int *)(void *)0;          /* no ace */
int x30 = (0)? 0: (0, (int *)(void *)0-(int *)(void *)0);    /* no ace */
int x31 = (0)? 0: (int *)0 - (int *)0;                       /* no ace */
int x32 = 1 && (int *)0 - (int *)0;                          /* no ace */
int x33 = (0)? 0: (0, (int *)0-(int *)0);                    /* no ace */

int x34 = (1)? (int)(void *)1: 0;         /* no ace */
int x35 = (1)? (1, (int)(void *)1): 0;    /* no ace */

int *x36 = (0)? 0: (int *)(int)(void *)1;                 /* no addr */
int *x37 = (0)? 0: ((void *)0, (int *)(int)(void *)1);    /* no addr */

int *x38 = (void *)&x28;
int *x39 = (int *)(char *)&x28;
