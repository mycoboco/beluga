/* -Wv --std=c90 */

/* all okay */
int f(void)
{
    int x1[sizeof(3.14 + 3.14)];
    int x2[sizeof(3.14 - 1.14)];
    int x3[sizeof(3.14 * 3.14)];
    int x4[sizeof(3.14 / 3.14)];
    int x5[sizeof(-3.14)];
    int x6[sizeof(+3.14)];

    enum {
        A = sizeof(!3.14),
        B = sizeof(3.14 && 3.14),
        C = sizeof(3.14 || 3.14),
        D = sizeof(3.14 == 3.14),
        E = sizeof(3.14 != 3.14)
    };

    struct tag {
        unsigned  : sizeof(3.14 >  3.14);
        unsigned b: sizeof(3.14 >= 3.14);
    };

    switch(f()) {
        case sizeof(3.14 <  3.14):   break;
        case sizeof(3.14 <= 3.14)+1: break;

        case sizeof((3.14)? 10: 0)-1:       break;
        case sizeof((1)? 20: 3.14):         break;
        case sizeof((1)? 30: (double)3)-2:  break;
        case sizeof((1)? 40.0: 0)-1:        break;
        case sizeof((1)? (double)50: 0)+1:  break;
        case sizeof(((int)3.14)? 60: 0)+10: break;
        case sizeof((1)? (int)70.0: 0)+11:  break;
    }

    return 0;
}


int g(void)
{
    int x1[sizeof(int [(int)(3.14 + 3.14)])];
    int x2[sizeof(int [(int)(3.14 - 1.14)])];
    int x3[sizeof(int [(int)(3.14 * 3.14)])];
    int x4[sizeof(int [(int)(3.14 / 3.14)])];
    int x5[sizeof(int [-(int)-3.14])];
    int x6[sizeof(int [(int)+3.14])];

    int x7[sizeof(int [(unsigned)(3.14 + 3.14)])];
    int x8[sizeof(int [(unsigned)(3.14 - 1.14)])];
    int x9[sizeof(int [(unsigned)(3.14 * 3.14)])];
    int x10[sizeof(int [(unsigned)(3.14 / 3.14)])];
    int x11[sizeof(int [-(unsigned)-3.14])];
    int x12[sizeof(int [(unsigned)+3.14])];

    enum {
        A = sizeof(int [!3.14+1]),
        B = sizeof(int [3.14 && 3.14]),
        C = sizeof(int [3.14 || 3.14]),
        D = sizeof(int [3.14 == 3.14]),
        E = sizeof(int [3.14 != 3.12])
    };

    struct tag {
        unsigned a: sizeof(int [3.14 >  3.12]);
        unsigned b: sizeof(int [3.14 >= 3.14]);
    };

    switch(f()) {
        case sizeof(int [3.12 <  3.14]):   break;
        case sizeof(int [3.14 <= 3.14])+1: break;

        case sizeof(int [(1)? 20: (int)3.14]):      break;    /* okay */
        case sizeof(int [(1)? 30: (int)(double)3]): break;
        case sizeof(int [(1)? (int)40.0: 0]):       break;    /* okay */
        case sizeof(int [(1)? (int)(double)50: 0]): break;
    }

    switch(f()) {
        case sizeof(int [(1)? 20: (unsigned)3.14]):      break;    /* okay */
        case sizeof(int [(1)? 30: (unsigned)(double)3]): break;
        case sizeof(int [(1)? (unsigned)40.0: 0]):       break;    /* okay */
        case sizeof(int [(1)? (unsigned)(double)50: 0]): break;
    }

    return 0;
}


int h(void)
{
    int x1[(int)(sizeof(int [(int)(3.14 + 3.14)]) + 3.14)];
    int x2[(int)(sizeof(int [(int)(3.14 - 1.14)]) - 3.14)];
    int x3[(int)(sizeof(int [(int)(3.14 * 3.14)]) * 3.14)];
    int x4[(int)(sizeof(int [(int)(3.14 / 3.14)]) / 3.14)];
    int x5[(int)(sizeof(int [-(int)-3.14]) + (int)-3.14)];
    int x6[(int)(sizeof(int [(int)+3.14]) + (int)+3.14)];

    int x7[(unsigned)(sizeof(int [(unsigned)(3.14 + 3.14)]) + 3.14)];
    int x8[(unsigned)(sizeof(int [(unsigned)(3.14 - 1.14)]) - 3.14)];
    int x9[(unsigned)(sizeof(int [(unsigned)(3.14 * 3.14)]) * 3.14)];
    int x10[(unsigned)(sizeof(int [(unsigned)(3.14 / 3.14)]) / 3.14)];
    int x11[(unsigned)(sizeof(int [-(unsigned)-3.14]) + (unsigned)-3.14)];
    int x12[(unsigned)(sizeof(int [(unsigned)+3.14]) + (unsigned)+3.14)];

    enum {
        A = sizeof(int [!3.14+1]) + !3.14,
        B = sizeof(int [3.14 && 3.14]) && 3.14,
        C = sizeof(int [3.14 || 3.14]) || 3.14,
        D = sizeof(int [3.14 == 3.14]) == 3.14,
        E = sizeof(int [3.14 != 3.12]) != 3.14
    };

    struct tag {
        unsigned a: sizeof(int [3.14 >  3.12]) >  3.14;
        unsigned b: sizeof(int [3.14 >= 3.14]) >= 3.14;
    };

    switch(f()) {
        case sizeof(int [3.12 <  3.14]) <  3.14:     break;
        case 1+(sizeof(int [3.14 <= 3.14]) <= 3.14): break;

        case sizeof((3.14)? 10: 0) + ((3.14)? 10: 0):                  break;
        case sizeof((1)? 20: 3.14) + ((int)((1)? 20: 3.14)):           break;
        case sizeof((1)? 30: (double)3) + ((int)((1)? 30: (double)3)): break;
        case sizeof((1)? 40.0: 0) + ((int)((1)? 40.0: 0)):             break;
        case sizeof(((int)3.14)? 60: 0) + (((int)3.14)? 60: 0):        break;    /* okay */
        case sizeof((1)? (int)70.0: 0) + ((1)? (int)70.0: 0):          break;    /* okay */
    }

    switch(f()) {
        case sizeof((1)? 20: 3.14) + ((unsigned)((1)? 20: 3.14)):           break;
        case sizeof((1)? 30: (double)3) + ((unsigned)((1)? 30: (double)3)): break;
        case sizeof((1)? 40.0: 0) + ((unsigned)((1)? 40.0: 0)):             break;
        case sizeof(((unsigned)3.14)? 60: 0) + (((unsigned)3.14)? 60: 0):   break;    /* okay */
        case sizeof((1)? (unsigned)70.0: 0) + ((1)? (unsigned)70.0: 0):     break;    /* okay */
    }

    return 0;
}
