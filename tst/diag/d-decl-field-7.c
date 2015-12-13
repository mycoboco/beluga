struct t1 {
    int x: 4;
    int y: 7;
    char c[2];
};

void f1(void)
{
    1 / (sizeof(struct t1) - 4);    /* warning if byte endian == bit endian */
    1 / (sizeof(struct t1) - 8);    /* warning if byte endian != bit endian */

    1 / ((int)&((struct t1 *)0)->c - 2);    /* warning if byte endian == bit endian */
    1 / ((int)&((struct t1 *)0)->c - 4);    /* warning if byte endian != bit endian */
}

struct t2 {
    int x: 4;
    int y: 7;
    int :0;
    char c[2];
};

void f2(void)
{
    1 / (sizeof(struct t2) - 8);
    1 / ((int)&((struct t2 *)0)->c - 4);
}

struct t3 {
    int x: 4;
    int y: 7;
    int :0;
    int z: 1;
    char c[2];
};

void f3(void)
{
    1 / (sizeof(struct t2) - 8);    /* warning if byte endian == bit endian */
    1 / (sizeof(struct t2) - 12);    /* warning if byte endian != bit endian */

    1 / ((int)&((struct t3 *)0)->c - 5);    /* warning if byte endian == bit endian */
    1 / ((int)&((struct t3 *)0)->c - 8);    /* warning if byte endian != bit endian */
}
