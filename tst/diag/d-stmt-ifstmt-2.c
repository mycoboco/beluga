int x;

void f(void)
{
    if (x)
        if (x)
            f();

    if (x)
        if (x)
            f();
        else
            f();

    if (x)
        if (x)
            f();
        else
            f();
    else
        f();

    if (x)
        if (x)
            f();
    else
        f();

    if (x) {
        if (x)
            f();
    } else
        f();

    if (x)
        for (;x;)
            if (x)
                f();
            else
                f();

    if (x)
        for (;x;)
            if (x)
                f();
    else
        f();

    if (x) {
        for (;x;)
            if (x)
                f();
    } else
        f();

    if (x)
        while (x)
            if (x)
                f();
    else
        f();

    if (x)
        do
            if (x)
                f();
    else
        f();
        while(x);

    if (x)
        switch(x)
            if (x)
                f();
            else
                f();

    if (x)
        switch(x)
            if (x)
                f();
    else
        f();

    if (x)
        switch(x)
            default:
            case 0:
                if (x)
                    f();
    else
        f();

    if (x)
        label1:
            if (x)
                f();
    else
        f();

    if (x) {
        label2:
            if (x)
                f();
    } else
        f();

	if (x)
		if (x)
			f();
		else
			f();

	if (x)
		if (x)
			f();
        else
			f();

	if (x)
		if (x)
			f();
	else
		f();
}
