int x;

void f(void)
{
    if (x)
        if (x)
            if (x)
                f();
            else f();

    if (x)
        if (x)
            if (x)
                f();
            else
                f();
        else
            f();

    if (x)
        if (x)
            if (x)
                f();
            else
                f();
        else
            f();
    else
        f();

    if (x)
        if (x)    /* warning */
            if (x)
                f();
        else f();

    if (x)
        if (x)    /* warning */
            for (;x;)
                if (x)
                    f();
        else f();

    if (x)
        if (x)
            do
                if (x)
                    f();
        else f();
            while(x);

    if (x)    /* warning */
        if (x) {
            if (x)
                f();
        } else f();

    if (x)    /* warning */
        if (x)
            if (x)
                f();
            else
                f();
    else
        f();

    if (x)    /* warning */
        switch(x)
            case 0:
            default:
                if (x)
                    if (x)
                        f();
                    else
                        f();
    else
        f();

    if (x) {
        if (x)
            if (x)
                f();
            else
                f();
    } else
        f();

    if (x)    /* warning */
        if (x)
            if (x)
                f();
        else
            f();
    else
        f();

    if (x)    /* warning */
        label:
            if (x)
                if (x)
                    f();
            else
                f();
    else
        f();

    if (x) {
        if (x)    /* warning */
            if (x)
                f();
        else
            f();
    } else
        f();

	if (x)
		if (x)
			if (x)
				f();
			else
				f();
		else
			f();

	if (x)    /* warning */
		if (x)
			if (x)
				f();
			else
				f();
        else
			f();

	if (x) {
		if (x)
			if (x)
				f();
			else
				f();
	} else
		f();
}
