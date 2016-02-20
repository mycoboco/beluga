int f(void)
{
    int x;
    struct tag { struct tag *m; } z(), y;

    +((f())? 0: 1);
    +((f())? (int)(x = 0): 1);
    +((f())? 0: (int)(x = 0));
    +((f())? (int)(x = 0): (int)(x = 0));
    +((f())? 0: (void)(x = 0));
    +((f())? (void)(x = 0): (void)(x = 0));
    +((f())? (x = 0): (x = 1));

    +((f())? 0: 1);
    +((f())? f(): 1);
    +((f())? (int)f(): 1);
    +((f())? 0: f());
    +((f())? 0: (int)f());
    +((f())? f(): f());
    +((f())? (int)f(): (int)f());
    +((f())? 0: (void)f());
    +((f())? (void)f(): (void)f());

    (f())? 0: 1;
    (f())? (int)(x = 0): 1;
    (f())? 0: (int)(x = 0);
    (f())? (int)(x = 0): (int)(x = 0);
    (f())? 0: (void)(x = 0);
    (f())? (void)(x = 0): (void)(x = 0);    /* no warning */
    (f())? (x = 0): (x = 1);                /* no warning */

    (f())? 0: 1;
    (f())? f(): 1;                  /* no warning */
    (f())? (int)f(): 1;
    (f())? 0: f();                  /* no warning */
    (f())? 0: (int)f();
    (f())? f(): f();                /* no warning */
    (f())? (int)f(): (int)f();
    (f())? 0: (void)f();
    (f())? (void)0: (void)f();      /* no warning */
    (f())? (void)f(): (void)f();    /* no warning */

    (f())? +(x = 0): +(x = 0);
    (f())? -(x = 0): -(x = 0);

    (f())? (1, (int)f()): (int)f();

    (f())? z(): *(y.m);    /* no warning */
}
