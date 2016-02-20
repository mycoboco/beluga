/* -Wv --std=c90 */

int f(void)
{
    int x1[(int)(3.14 + 3.14)];
    int x2[(int)(3.14 - 1.14)];
    int x3[(int)(3.14 * 3.14)];
    int x4[(int)(3.14 / 3.14)];
    int x5[-(int)-3.14];
    int x6[(int)+3.14];

    int x7[(unsigned)(3.14 + 3.14)];
    int x8[(unsigned)(3.14 - 1.14)];
    int x9[(unsigned)(3.14 * 3.14)];
    int x10[(unsigned)(3.14 / 3.14)];
    int x11[-(unsigned)-3.14];
    int x12[(unsigned)+3.14];

    enum {
        A = !3.14,
        B = 3.14 && 3.14,
        C = 3.14 || 3.14,
        D = 3.14 == 3.14,
        E = 3.14 != 3.14
    };

    struct tag {
        unsigned  : 3.14 >  3.14;
        unsigned b: 3.14 >= 3.14;
    };

    switch(f()) {
        case 3.14 <  3.14: break;
        case 3.14 <= 3.14: break;

        case (3.14)? 10: 0:             break;
        case (int)((1)? 20: 3.14):      break;
        case (int)((1)? 30: (double)3): break;
        case (int)((1)? 40.0: 0):       break;
        case (int)((1)? (double)50: 0): break;
        case ((int)3.14)? 60: 0:        break;    /* okay */
        case (1)? (int)70.0: 0:         break;    /* okay */
    }

    switch(f()) {
        case (unsigned)((1)? 20: 3.14):      break;
        case (unsigned)((1)? 30: (double)3): break;
        case (unsigned)((1)? 40.0: 0):       break;
        case (unsigned)((1)? (double)50: 0): break;
        case ((unsigned)3.14)? 60: 0:        break;    /* okay */
    }

    return 0;
}


int g(void)
{
    int x1[(int)3.14 + (int)3.14];
    int x2[(int)3.14 - (int)1.14];
    int x3[(int)3.14 * (int)3.14];
    int x4[(int)3.14 / (int)3.14];
    int x5[- -(int)3.14];
    int x6[+(int)3.14];

    int x7[(unsigned)3.14 + (unsigned)3.14];
    int x8[(unsigned)3.14 - (unsigned)1.14];
    int x9[(unsigned)3.14 * (unsigned)3.14];
    int x10[(unsigned)3.14 / (unsigned)3.14];
    int x11[- -(unsigned)3.14];
    int x12[+(unsigned)3.14];

    enum {
        A = !(int)3.14,
        B = (int)3.14 && (int)3.14,
        C = (int)3.14 || (int)3.14,
        D = (int)3.14 == (int)3.14,
        E = (int)3.14 != (int)3.14
    };

    enum {
        F = !(unsigned)3.14,
        G = (unsigned)3.14 && (unsigned)3.14,
        H = (unsigned)3.14 || (unsigned)3.14,
        I = (unsigned)3.14 == (unsigned)3.14,
        J = (unsigned)3.14 != (unsigned)3.14
    };

    struct tag {
        unsigned  : (int)3.14 >  (int)3.14;
        unsigned b: (int)3.14 >= (int)3.14;

        unsigned  : (unsigned)3.14 >  (unsigned)3.14;
        unsigned d: (unsigned)3.14 >= (unsigned)3.14;
    };

    switch(f()) {
        case (int)3.14 <  (int)3.14: break;
        case (int)3.14 <= (int)3.14: break;

        case (1)? 20: (int)3.14:      break;
        case (1)? 30: (int)(double)3: break;
        case (1)? (int)40.0: 0:       break;
        case (1)? (int)(double)50: 0: break;
    }

    switch(f()) {
        case (unsigned)3.14 <  (unsigned)3.14: break;
        case (unsigned)3.14 <= (unsigned)3.14: break;

        case (1)? 20: (unsigned)3.14:      break;
        case (1)? 30: (unsigned)(double)3: break;
        case (1)? (unsigned)40.0: 0:       break;
        case (1)? (unsigned)(double)50: 0: break;
    }

    return 0;
}


int i;
double d;


int h(void)
{
    int x1[(int)(d + 3.14)];
    int x2[(int)(3.14 - d)];
    int x3[(int)(d * 3.14)];
    int x4[(int)(3.14 / d)];

    int x5[(unsigned)(d + 3.14)];
    int x6[(unsigned)(3.14 - d)];
    int x7[(unsigned)(d * 3.14)];
    int x8[(unsigned)(3.14 / d)];

    enum {
        B = d && 3.14,
        C = 3.14 || d,
        D = d == 3.14,
        E = 3.14 != d
    };

    struct tag {
        unsigned  : d >  3.14;
        unsigned b: 3.14 >= d;
    };

    switch(f()) {
        case d <  3.14: break;
        case 3.14 <= d: break;

        case (int)((1)? d: 3.14):       break;
        case (int)((1)? d: (double)3):  break;
        case (int)((1)? 40.0: d):       break;
        case (int)((1)? (double)50: d): break;
        case (1)? (int)70.0: (int)d:    break;
    }

    switch(f()) {
        case (unsigned)((1)? d: 3.14):         break;
        case (unsigned)((1)? d: (double)3):    break;
        case (unsigned)((1)? 40.0: d):         break;
        case (unsigned)((1)? (double)50: d):   break;
        case (1)? (unsigned)70.0: (unsigned)d: break;
    }

    return 0;
}


/* no ice */
int j(void)
{
    int x1[(int)(d + d)];
    int x2[(int)(d - d)];
    int x3[(int)(d * d)];
    int x4[(int)(d / d)];
    int x5[-(int)-d];
    int x6[(int)+d];

    int x7[(unsigned)(d + d)];
    int x8[(unsigned)(d - d)];
    int x9[(unsigned)(d * d)];
    int x10[(unsigned)(d / d)];
    int x11[-(unsigned)-d];
    int x12[(unsigned)+d];

    enum {
        A = !d,
        B = d && d,
        C = d || d,
        D = d == d,
        E = d != d
    };

    struct tag {
        unsigned  : d >  d;
        unsigned b: d >= d;
    };

    switch(f()) {
        case d <  d: break;
        case d <= d: break;

        case (d)? 10: 0:                break;
        case (int)((1)? 20: d):         break;
        case (int)((1)? 30: (double)i): break;
        case (int)((1)? d: 0):          break;
        case (int)((1)? (double)i: 0):  break;
        case ((int)d)? 60: 0:           break;
        case (1)? (int)d: 0:            break;
    }

    switch(f()) {
        case (unsigned)((1)? 20: d):         break;
        case (unsigned)((1)? 30: (double)i): break;
        case (unsigned)((1)? d: 0):          break;
        case (unsigned)((1)? (double)i: 0):  break;
        case ((unsigned)d)? 60: 0:           break;
        case (1)? (unsigned)d: 0:            break;
    }

    return 0;
}
