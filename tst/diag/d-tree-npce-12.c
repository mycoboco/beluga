/* -Wv --std=c90 */

int f(void);

int i1 = 0 && (1, 2);
int *i2 = (int *)(1 || (i1, i2));                    /* okay */
int *i3 = (int *)(0 && (&i1+(int)(3.14+3), &i1));    /* okay */

int i4 = 0 && ((int (*)())0)();
int i5 = 0 && f();
int i6 = 1 || ++*((int *)0);
int i7 = 1 || i6++;
int i8 = (1)? 0: (*((int *)0) = 0);
int i9 = (1)? 0: (i6 = 0);
int i10 = 0 && (*((int *)0) += 0);
int i11 = 0 && (i6 += 0);
int *i12 = (int *)(0 && ((int (*)())0)());
int *i13 = (int *)(1 || ++*((int *)0));
int *i14 = (int *)((1)? 0: (*((int *)0) = 0));
int *i15 = (int *)(0 && (*((int *)0) += 0));
int i16 = sizeof(0 && ((int (*)())0)());          /* okay */
int i17 = sizeof(1 || ++*((int *)0));             /* okay */
int i18 = sizeof((1)? 0: (*((int *)0) = 0));      /* okay */
int i19 = sizeof(0 && (*((int *)0) += 0));        /* okay */

int i20 = 1 || &i1 == 0;
int i21 = 1 || ((void *)0, 1);
int i22 = 0 && ((int)(void *)0 + 3.14);

int i23 = (1, 2) || 0;
int *i24 = (int *)((i1, i2) || 1);
int *i25 = (int *)((&i1+(int)(3.14+3)) && 0);
int i26 = ((int (*)())0)() && 0;
int i27 = ++*((int *)0) && 0;
int i28 = (*((int *)0) = 0) || 1;
int i29 = (*((int *)0) += 0) || 1;

int *i30 = &i1 + (&i1 == 0);
int i31 = 3.14 * 2 + (&i1 != 0);
