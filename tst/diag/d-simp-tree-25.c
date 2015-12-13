void f(void)
{
    int x;

    x >> 0xffffffff;                   /* issued */
    x << 0xffffffff;                   /* issued */
    x >> 0x7fffffffL;                  /* issued */
    x << 0x7fffffffL;                  /* issued */
    x >> (unsigned char)0xffffff01;
    x << (unsigned char)0xffffffff;    /* okay */
    x >> (signed char)0xffffff01;
    x << (signed char)0xffffffff;
    x >> (unsigned)0xffffff01;         /* issued */
    x << (unsigned)0xffffffff;         /* issued */
    x >> (unsigned)0x0fffff01;         /* issued */
    x << (unsigned)0x0fffffff;         /* issued */
    x >> (short)0x0fffff01;
    x << (short)0x0fffffff;
    x >> (int)31.9;                    /* okay */
    x >> (short)31.9;                  /* okay */
    x << (unsigned char)31.9;          /* okay */
    x >> (int)32.5;                    /* issued */
    x << (int)33.0;                    /* issued */
    x >> (short)32.5;                  /* issued */
    x << (short)33.0;                  /* issued */
    x << (int)3.14e10;                 /* issued */
    x << (unsigned)3.14e10;
    x << (short)3.14e10;
    x << -(int)3.14e10;
    x << (int)-3.14e10;                /* issued */
    x << (unsigned)-3.14e10;
    x << (short)-3.14e10;
    x >> (1UL << 31);                  /* issued */
    x << (unsigned long)-1;            /* issued */
    x << (unsigned char)-1;            /* issued */
    x << (short)-1;                    /* issued */
    x >> (int)3.14e-10;                /* okay */
    x >> (short)3.14e-10;              /* okay */
}
