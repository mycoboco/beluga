int x;

void f(void)
{
    if (1);
    if (1) /* ... */ ;
    if (1)
        ;

    while (x);
    while (x) /* ... */ ;
    while (x)
        ;

    for (; x;);
    for (; x;) /* ... */ ;
    for (; x;)
        ;

    do; while(0);
    do /* ... */ ; while(0);
    do
    ; while(0);
    do;
    while(0);
    do
    ;
    while(0);
}
