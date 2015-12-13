void f(void)
{
    L1:
        goto L2;
    L1:    /* invalid */
        goto L3;
    {
        L2:
            f();
        L3:
            f();
    }
}
