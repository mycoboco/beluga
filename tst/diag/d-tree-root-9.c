void f(void)
{
    float fx;
    double dx;
    long double ldx;

    unsigned u;
    unsigned long ul;

    (unsigned)(fx = 0);
    (unsigned)(dx = 0);
    (unsigned)(ldx = 0);

    (unsigned)((fx = 0) + (fx = 0));
    (unsigned)((dx = 0) + (dx = 0));
    (unsigned)((ldx = 0) + (ldx = 0));

    (unsigned)((float)(unsigned)(fx = 0));
    (unsigned)((double)(unsigned)(dx = 0));
    (unsigned)((long double)(unsigned)(ldx = 0));

    (unsigned long)(fx = 0);
    (unsigned long)(dx = 0);
    (unsigned long)(ldx = 0);

    (unsigned long)((fx = 0) + (fx = 0));
    (unsigned long)((dx = 0) + (dx = 0));
    (unsigned long)((ldx = 0) + (ldx = 0));

    (unsigned long)((float)(unsigned long)(fx = 0));
    (unsigned long)((double)(unsigned long)(dx = 0));
    (unsigned long)((long double)(unsigned long)(ldx = 0));


    (float)(u = 0);
    (double)(u = 0);
    (long double)(u = 0);

    (float)((u = 0) + (u = 0));
    (double)((u = 0) + (u = 0));
    (long double)((u = 0) + (u = 0));

    (float)((unsigned)(float)(u = 0));
    (double)((unsigned)(long)(u = 0));
    (long double)((unsigned)(long double)(u = 0));

    (float)(ul = 0);
    (double)(ul = 0);
    (long double)(ul = 0);

    (float)((ul = 0) + (ul = 0));
    (double)((ul = 0) + (ul = 0));
    (long double)((ul = 0) + (ul = 0));

    (float)((unsigned long)(float)(ul = 0));
    (double)((unsigned long)(long)(ul = 0));
    (long double)((unsigned long)(long double)(ul = 0));
}

void g(void)
{
    float fy();
    double dy();
    long double ldy();

    unsigned uy();
    unsigned long uly();

    (unsigned)fy();
    (unsigned)dy();
    (unsigned)ldy();

    (unsigned)(fy() + fy());
    (unsigned)(dy() + dy());
    (unsigned)(ldy() + ldy());

    (unsigned)((float)(unsigned)fy());
    (unsigned)((double)(unsigned)dy());
    (unsigned)((long double)(unsigned)ldy());

    (unsigned long)fy();
    (unsigned long)dy();
    (unsigned long)ldy();

    (unsigned long)(fy() + fy());
    (unsigned long)(dy() + dy());
    (unsigned long)(ldy() + ldy());

    (unsigned long)((float)(unsigned long)fy());
    (unsigned long)((double)(unsigned long)dy());
    (unsigned long)((long double)(unsigned long)ldy());


    (float)uy();
    (double)uy();
    (long double)uy();

    (float)(uy() + uy());
    (double)(uy() + uy());
    (long double)(uy() + uy());

    (float)((unsigned)(float)uy());
    (double)((unsigned)(double)uy());
    (long double)((unsigned)(long double)uy());

    (float)uly();
    (double)uly();
    (long double)uly();

    (float)(uly() + uly());
    (double)(uly() + uly());
    (long double)(uly() + uly());

    (float)((unsigned long)(float)uly());
    (double)((unsigned long)(double)uly());
    (long double)((unsigned long)(long double)uly());
}
