void f(void)
{
    (1)? (1 + 1): (void *)0;
    (1)? (1U + 1U): (void *)0;
    (1)? (1L + 1L): (void *)0;
    (1)? (1UL + 1UL): (void *)0;

    (1)? (1 + 1L): (void *)0;
    (1)? (1U + 1UL): (void *)0;

    (1)? (1 + 1U): (void *)0;
    (1)? (1L + 1UL): (void *)0;

    (1)? (1L + 1U): (void *)0;
}
