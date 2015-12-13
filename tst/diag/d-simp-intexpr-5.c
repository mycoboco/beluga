/* -Wv --std=c90 */

struct m {
    int x;
    int y;
} q;


/* all okay */
int f(void)
{
    int x1[sizeof((void *)0)];
    int x2[sizeof((char *)0)];
    int x3[sizeof((char *)1)];

    enum {
        A = sizeof((char *)1 - (char *)0),
        B = sizeof(!(char *)1),
        C = sizeof((char *)1 && (char *)0),
        D = sizeof((char *)1 || (char *)0),
        E = sizeof((char *)1 == (char *)0),
        F = sizeof((char *)1 != (char *)0)
    };

    struct tag {
        unsigned  : sizeof((char *)1 >  (char *)0);
        unsigned b: sizeof((char *)1 >= (char *)0);
    };

    switch(f()) {
        case sizeof((char *)1 <  (char *)0):      break;
        case sizeof((char *)1 <= (char *)0) + 10: break;

        case sizeof(((void *)1)? 10: 0) + 20:           break;
        case sizeof((1)? 0: (void *)0) + 30:            break;
        case sizeof((1)? (void *)0: 0) + 40:            break;
        case sizeof((1)? (int)(void *)70: 0) + 50:      break;
        case sizeof((1)? (unsigned)(void *)70: 0) + 60: break;
    }

    {
        int y1[sizeof((unsigned)(&((struct m *)0)->y))];    /* offsetof */
        int y2[sizeof((int)(&((struct m *)0)->y))];         /* offsetof */
    }

    return 0;
}


int g(void)
{
    int x1[sizeof(int [(int)(void *)0 + 1])];
    int x2[sizeof(int [(int)(char *)0 + 2])];
    int x3[sizeof(int [(int)(char *)1])];

    int x4[sizeof(int [(unsigned)(void *)0 + 1])];
    int x5[sizeof(int [(unsigned)(char *)0 + 2])];
    int x6[sizeof(int [(unsigned)(char *)1])];

    enum {
        A = sizeof(int [(char *)1 - (char *)0]),
        B = sizeof(int [!!(char *)1]),
        C = sizeof(int [((char *)1 && (char *)0)+1]),
        D = sizeof(int [(char *)1 || (char *)0]),
        E = sizeof(int [((char *)1 == (char *)0)+1]),
        F = sizeof(int [(char *)1 != (char *)0])
    };

    struct tag {
        unsigned  : sizeof(int [(char *)1 >  (char *)0]);
        unsigned b: sizeof(int [(char *)1 >= (char *)0]);
    };

    switch(f()) {
        case sizeof(int [((char *)1 <  (char *)0)+1]):      break;
        case sizeof(int [((char *)1 <= (char *)0)+1]) + 10: break;

        case sizeof(int [((void *)1)? 10: 0]) + 20:           break;
        case sizeof(int [((int)((1)? 0: (void *)0))+1]) + 30: break;
        case sizeof(int [((int)((1)? (void *)0: 0))+1]) + 40: break;
        case sizeof(int [(1)? (int)(void *)70: 0]) + 50:      break;
    }

    switch(f()) {
        case sizeof(int [((unsigned)((1)? 0: (void *)0))+1]) + 30: break;
        case sizeof(int [((unsigned)((1)? (void *)0: 0))+1]) + 40: break;
        case sizeof(int [(1)? (unsigned)(void *)70: 0]) + 50:      break;
    }

    {
        int y1[sizeof(int [(unsigned)(&((struct m *)0)->y)])];    /* offsetof */
        int y2[sizeof(int [(int)(&((struct m *)0)->y)])];         /* offsetof */
    }

    return 0;
}


int h(void)
{
    int x1[sizeof(int [(int)(void *)0 + 1]) + (int)(void *)0];
    int x2[sizeof(int [(int)(char *)0 + 2]) + (int)(char *)0];
    int x3[sizeof(int [(int)(char *)1]) + (int)(char *)1];

    int x4[sizeof(int [(unsigned)(void *)0 + 1]) + (unsigned)(void *)0];
    int x5[sizeof(int [(unsigned)(char *)0 + 2]) + (unsigned)(char *)0];
    int x6[sizeof(int [(unsigned)(char *)1]) + (unsigned)(char *)1];

    enum {
        A = sizeof(int [(char *)1 - (char *)0]) + ((char *)1 - (char *)0),
        B = sizeof(int [!!(char *)1]) + !(char *)1,
        C = sizeof(int [((char *)1 && (char *)0)+1]) + ((char *)1 && (char *)0),
        D = sizeof(int [(char *)1 || (char *)0]) + ((char *)1 || (char *)0),
        E = sizeof(int [((char *)1 == (char *)0)+1]) + ((char *)1 == (char *)0),
        F = sizeof(int [(char *)1 != (char *)0]) + ((char *)1 != (char *)0)
    };

    struct tag {
        unsigned  : sizeof(int [(char *)1 >  (char *)0]) + ((char *)1 >  (char *)0);
        unsigned b: sizeof(int [(char *)1 >= (char *)0]) + ((char *)1 >= (char *)0);
    };

    switch(f()) {
        case sizeof(int [((char *)1 <  (char *)0)+1]) + ((char *)1 <  (char *)0):      break;
        case sizeof(int [((char *)1 <= (char *)0)+1]) + 10 + ((char *)1 <= (char *)0): break;

        case sizeof(int [((void *)1)? 10: 0]) + 20 + (((void *)1)? 10: 0):                 break;
        case sizeof(int [((int)((1)? 0: (void *)0))+1]) + 30 + ((int)((1)? 0: (void *)0)): break;
        case sizeof(int [((int)((1)? (void *)0: 0))+1]) + 40 + ((int)((1)? (void *)0: 0)): break;
        case sizeof(int [(1)? (int)(void *)70: 0]) + 50 + ((1)? (int)(void *)70: 0):       break;
    }

    switch(f()) {
        case sizeof(int [((unsigned)((1)? 0: (void *)0))+1]) + 30 + ((unsigned)((1)? 0: (void *)0)): break;
        case sizeof(int [((unsigned)((1)? (void *)0: 0))+1]) + 40 + ((unsigned)((1)? (void *)0: 0)): break;
        case sizeof(int [(1)? (unsigned)(void *)70: 0]) + 50 + ((1)? (unsigned)(void *)70: 0):       break;
    }

    {
        int y1[sizeof(int [(unsigned)(&((struct m *)0)->y)]) +
               (unsigned)(&((struct m *)0)->y)];    /* offsetof */
        int y2[sizeof(int [(int)(&((struct m *)0)->y)]) +
               (int)(&((struct m *)0)->y)];    /* offsetof */
    }

    return 0;
}
