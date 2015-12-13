/* -Wv */

int q;

/* warnings */
int x1[((1 + (int)(void *)0 * 2 % 1) & 0xff | 1) << 2 >> 1];
int x2[(~(unsigned long)+3.14 & 0xff) ^ 0 ^ 0];
int x3[((~(int)3.14 & 0xff) ^ 0 ^ 0) - (int)-3.14];

/* errors */
void f(void)
{
    int y1[((int (*)())0)(1)];
    int y2[((int (*)())0)()];
    int y3[((int (*)())0)(q, q)];

    int z1[((int *)0) = 1];
    int z2[*((int *)0) = 1];
    int z3[*((void *)0) = 1];
    int z4[*((int *)1) = 1];
    int z5[((int *)0)[1] = 2];
    int z6[((int *)1)[0] = 2];

    enum {
        A = *((unsigned *)0),
        B = *((double *)1),
        C = ((unsigned *)0)[1],
        D = ((double *)1)[2]
    };

    switch(q) {
        case (*((int *)0))++: break;
        case (*((int *)1))--: break;
    }
}

void g(void)
{
    struct tag {
        int m;
    };

    /* warnings */
    int y1[&(*((int *)0)) == 0];
    int y2[(&(*((int *)1)) == 0)+1];

    /* errors */
    int w1[((struct tag *)0)->m];
    int w2[(*((struct tag *)0)).m];

    /* warnings */
    enum {
        A = 0 && (*((int *)0) = 1),
        B = 0 && (((unsigned *)0)[1]),
        C = 1 || (*((int *)0))++,
        D = 1 || &(*((int *)0)) == 0
    };

    switch(q) {
        /* warnings */
        case (1)? 2: ((struct tag *)0)->m: break;
        case (1)? 3: (*((struct tag *)0)).m: break;
        /* errors */
        case ((((int *)0)[1] = 2), 4):       break;
        case ((&(*((int *)1)) == 0), 5):     break;
    }
}
