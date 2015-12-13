void f()
{
    return;
    return;    /* error */
    return;
    return;
}

int g()
{
    return;
    return 1;    /* error */
    return;
    return 1;
}

int h()
{
    return 1;
    return;    /* error */
    return 1;
    return;
}

void i()
{
    return;
    return;    /* error */
    return 1;
    return;
}

void j()
{
    return;
    return 1;    /* error */
    return;
    return 1;
}
