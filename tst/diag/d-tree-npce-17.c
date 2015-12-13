int i1 = (int)(void *)0;
int i2 = (int)(char *)1;
int i3 = (int)&i1;

int i4 = (char *)0 - (char *)0;
int i5 = (char *)1 - (char *)0;
int i6 = &i1 - &i1;
int x = &i1 == 0;

int *i7 = &i1 + ((void *)0 == (void *)0);
int *i8 = (int *)((void *)0 == (void *)0);
int *i9 = &i1 + ((void *)1 == (void *)1);
int *i10 = (int *)((void *)1 == (void *)1);
int *i11 = &i1 + (&i1 == &i1);
int *i12 = (int *)(&i1 == &i1);
int *i13 = &i1 + (&i1 == 0);
int *i14 = (int *)(&i1 == 0);

int *i15 = (int *)0;             /* okay */
int *i16 = (int *)(void *)0;     /* okay */
int *i17 = (int *)1;             /* okay */
int *i18 = (1)? 0: (int *)0;     /* okay */
int *i19 = (1)? 0: (void *)0;    /* okay */
int *i20 = (1)? 1: (int *)0;
int *i21 = (1)? 1: (void *)0;
int *i22 = (1)? 0: &i1;          /* okay */
int *i23 = (1)? 1: &i1;
