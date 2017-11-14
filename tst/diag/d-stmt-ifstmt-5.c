#define ELSE foo(); else

int foo(void)
{
    if (foo())
        if (foo())
    ELSE
        foo();

    if (foo())
        if (foo())
        ELSE
            foo();

    if (foo())
		if (foo())    /* two tabs */
  ELSE                /* two spaces */
        foo();
}
