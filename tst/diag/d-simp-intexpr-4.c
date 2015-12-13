/* -Wv --std=c90 */

struct m {
    int x;
    int y;
} q;


int f(void)
{
    int x1[(int)(void *)0 + 1];
    int x2[(int)(char *)0 + 1];
    int x3[(int)(char *)1];

    int x4[(unsigned)(void *)0 + 1];
    int x5[(unsigned)(char *)0 + 1];
    int x6[(unsigned)(char *)1];

    enum {
        A = (char *)1 - (char *)0,
        B = !(char *)1,
        C = (char *)1 && (char *)0,
        D = (char *)1 || (char *)0,
        E = (char *)1 == (char *)0,
        F = (char *)1 != (char *)0
    };

    struct tag {
        unsigned  : (char *)1 >  (char *)0;
        unsigned b: (char *)1 >= (char *)0;
    };

    switch(f()) {
        case (char *)1 <  (char *)0:       break;
        case ((char *)1 <= (char *)0) + 1: break;

        case ((void *)1)? 10: 0:            break;
        case (int)((1)? 0: (void *)0) + 30: break;
        case (int)((1)? (void *)0: 0) + 20: break;
        case (1)? (int)(void *)70: 0:       break;
    }

    switch(f()) {
        case (unsigned)((1)? 0: (void *)0) + 30: break;
        case (unsigned)((1)? (void *)0: 0) + 20: break;
        case (1)? (unsigned)(void *)70: 0:       break;
    }

    {
        int y1[((unsigned)(&((struct m *)0)->y))];    /* offsetof */
        int y2[((int)(&((struct m *)0)->y))];         /* offsetof */
    }

    return 0;
}


int g(void)
{
    int x1[(int)(void *)0 + (int)&q];
    int x2[(int)&q + (int)(char *)0];
    int x3[(int)(char *)1 + (int)&q];

    int x4[(unsigned)(void *)0 + (unsigned)&q];
    int x5[(unsigned)&q + (unsigned)(char *)0];
    int x6[(unsigned)(char *)1 + (unsigned)&q];

    enum {
        A = (char *)1 - (char *)&q,
        B = &q && (char *)0,
        C = (char *)0 && &q,
        D = (char *)1 || &q,
        E = &q || (char *)1,
        F = (char *)&q == (char *)0,
        G = (char *)1 != (char *)&q
    };

    struct tag {
        unsigned  : (char *)&q >  (char *)0;
        unsigned b: (char *)1  >= (char *)&q;
    };

    switch(f()) {
        case (char *)&q <  (char *)0: break;
        case (char *)1  <= (char *)&q: break;

        case (int)((1)? &q: (void *)0):     break;
        case (int)((1)? (void *)0: &q):     break;
        case (1)? (int)(void *)70: (int)&q: break;
    }

    switch(f()) {
        case (unsigned)((1)? &q: (void *)0):          break;
        case (unsigned)((1)? (void *)0: &q):          break;
        case (1)? (unsigned)(void *)70: (unsigned)&q: break;
    }

    return 0;
}


int h(void)
{
    int x1[(int)&q];
    int x2[(int)&q];
    int x3[(int)&q];

    int x4[(unsigned)&q];
    int x5[(unsigned)&q];
    int x6[(unsigned)&q];

    enum {
        A = &q - &q,
        B = !&q,
        C = &q && &q,
        D = &q || &q,
        E = &q == &q,
        F = &q != &q
    };

    struct tag {
        unsigned  : &q >  &q;
        unsigned b: &q >= &q;
    };

    switch(f()) {
        case &q <  &q: break;
        case &q <= &q: break;

        case (&q)? 10: 0:       break;
        case (int)((1)? 0: &q): break;
        case (int)((1)? &q: 0): break;
        case (1)? (int)&q: 0:   break;
    }

    switch(f()) {
        case (unsigned)((1)? 0: &q): break;
        case (unsigned)((1)? &q: 0): break;
        case (1)? (unsigned)&q: 0:   break;
    }

    return 0;
}
