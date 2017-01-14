/* -Wv --std=c90 */

void ((ff))()
{
    label: goto label;
    while(100+1);
    for (;;);
    for (;100;);
    do ; while(100);
}

void ((gg))()
{
    float *xx;

    if 1-1 return;
    if (1-1 return;
    if (*xx) if (*xx) return; else return;
    if (*xx)
        if (*xx) return;
    else return;
    if (*xx)
        if (*xx) return;
        else return;
}

void ((hh))()
{
    int i;

    for i=0; i < 10; i++
        break;
    for (i=0; i < 10; i++
        break;
}
