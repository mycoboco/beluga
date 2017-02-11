struct { int m:2; } x;

void f(void)
{
    &x.m;
    &(x.m);
    &*&(x.m);

    x.m+0 = 0;
    (x.m+0)++;
    &(x.m+0);

    (int)x.m = 0;
    ((int)x.m)++;
    &((int)x.m);

    +x.m = 0;
    (+x.m)++;
    &(+x.m);

    (x.m | 0) = 0;
    (x.m | 0)++;
    &(x.m | 0);

    (- -x.m) = 0;
    (~ ~x.m) = 0;
    &(- -x.m);
}
