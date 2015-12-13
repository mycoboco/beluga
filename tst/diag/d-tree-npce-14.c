/* -Wv --std=c90 */

int x1[((void *)0)? 1: 2];
int x2 = ((void *)0)? 1: 0;
int *x3 = ((void *)0)? (void *)0: (void *)0;
int x4[(&x2)? 1: 2];
int x5 = (&x2)? 1: 0;
int *x6 = (&x2)? (void *)0: (void *)0;

int x7 = (0 && (1.0, 2))? 1: 0;
int x8 = (0 && ((void *)0, 2))? 1: 0;

int x9[(3.14)? 1: 2];
int x10[((int)3.14)? 1: 2];                                 /* okay */
int x11 = (1)? 1: (1, 0);
int *x12 = (int *)((1)? 1: ((int)&x8, 0));                  /* okay */
int x13 = (1)? 1: (((void (*)())0)(), 1);
int *x14 = (1)? (int *)0: (((int *(*)())0)(), (int *)0);
int x15 = (1)? 1: ((1)? 1: ((void *)0, 1));
int x16 = (1)? ((1)? 1: ((void *)0, 1)): 1;
int x17[(int)((1)? 1: 3.14)];
int x18 = (int)((1)? 0: (void *)0);
int x19 = (1)? 1: x7;
int *x20 = (1)? &x7: (int *)(int)(void *)0;                 /* okay */
int *x21 = (1)? &x7+(int)(3.14+0): &x7;
int *x22 = (!(void *)0)? &x7: 0;

int *x23 = &x7 + ((void *)0 == (void *)0);
int *x24 = &x7 + ((int *)0 != (int *)0);

int *x25 = &x7 - (int)(3.14+0);
