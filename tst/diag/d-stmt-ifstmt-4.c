int x;

void f(void)
{
    if (x)
        if (x)
            f();
        else if (x)
            f();
        else
            f();
    else
        f();

    if (x)
        if (x)
            f();
        else if (x)
            f();
        else
            f();

    if (x)    /* warning */
        if (x)
            f();
        else if (x)
            f();
    else
        f();

    if (x) {
        if (x)
            f();
        else if (x)
            f();
    } else
        f();

    if (x)    /* warning */
        for (;x;)
            if (x)
                f();
            else if (x)
                f();
    else
        f();

    if (x)    /* warning */
        switch(x)
            case 0:
            default:
                if (x)
                    f();
                else if (x)
                    f();
    else
        f();

    if (x)
        do
            if (x)
                f();
            else if (x)
                f();
    else
        f();
        while(x);

    if (x)    /* warning */
        if (x)
            f();
    else if (x)
        f();
    else
        f();

    if (x)    /* warning */
        for (;x;)
            if (x)
                f();
    else if (x)
        f();
    else
        f();

    if (x)    /* warning */
        switch(x)
            case 0:
            default:
                if (x)
                    f();
    else if (x)
        f();
    else
        f();

    if (x)
        do
            if (x)
                f();
    else if (x)
        f();
    else
        f();
        while(x);

    if (x) {
        if (x)
            f();
    } else if (x)
        f();
    else
        f();

    if (x)
        f();
    else if (x)    /* warning */
        if (x)
            f();
    else
        f();

    if (x)
        f();
    else if (x)
        if (x)
            f();
        else
            f();

    if (x)
        f();
    else if (x)    /* warning */
        while (x)
            label:
                if (x)
                    f();
    else
        f();

    if (x)
        f();
    else if (x) {
        if (x)
            f();
    } else
        f();

	if (x)
		if (x)
			f();
		else if (x)
			f();
		else
			f();

	if (x)    /* warning */
		if (x)
			f();
		else if (x)
			f();
        else
			f();
}
