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
        if (x)
            if (x)
                f();
        else f();

    if (x)
        if (x)
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

    if (x)
        if (x) {
            if (x)
                f();
        } else f();

    if (x)
        if (x)
            if (x)
                f();
            else
                f();
    else
        f();

    if (x)
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

    if (x)
        if (x)
            if (x)
                f();
        else
            f();
    else
        f();

    if (x)
        label:
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

	if (x) {
		if (x)
			if (x)
				f();
			else
				f();
	} else
		f();
}
