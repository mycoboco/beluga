int x;

void f(void)
{
    struct { int m, n; } y;
    for (;;) ;
    for (; &x;) ;
    for (; &y.m;) ;
    for (; &y.n;) ;
    for (; f;) ;
    for (; f+1;) ;
    for (; (int *)f+1;) ;
    for (; 1;) ;
    for (; 1*8==8;) ;

    while (1) ;
    while (&x) ;
    while (&y.m) ;
    while (&y.n) ;
    while (f) ;
    while (f+1) ;
    while ((int *)f+1) ;
    while (1*8==8) ;

    label:
    goto label;

    label2:
    ;
    int z;
    goto label2;

    do ; while(1);
    do ; while(&x);
    do ; while(&y.m);
    do ; while(&y.n);
    do ; while(f);
    do ; while(f+1);
    do ; while((int *)f+1);
    do ; while(1*8==8);
}
