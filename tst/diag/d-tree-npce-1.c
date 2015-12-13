extern int s;
extern double d;

/* invalid */
int *p1 = &s + (int)(3.14 + 3.14);
int *p2 = &s - (int)-3.14;
int *p3 = !3.14 + &s;
int *p4 = (3.14 || 3.14) + &s;
int *p5 = &s - (3.14 >= 3.14);
int *p6 = ((3.14)? 1: 0) + &s;
int *p7 = &s - (int)((1)? 30: (double)3);
int *p8 = &s + (unsigned)(3.14 - d);
int *p9 = (3.14 || d) + &s;
int *p10 = &s - (3.14 <= d);

void f(void)
{
    /* valid */
    int *a[] = {
        &s + sizeof(3.14 + 3.14),
        sizeof(!3.14) + &s,
        &s - sizeof(3.14 && 3.14),
        sizeof(3.14 <= 3.14) + &s,
        &s + sizeof((1)? (double)50: 0)
    };

    int b[] = {
        sizeof(&s + (int)(3.14 + 3.14)),
        sizeof(!3.14 + &s),
        sizeof(&s - (3.14 && 3.14)),
        sizeof((3.14 <= 3.14) + &s),
        sizeof(&s + (int)((1)? (double)50: 0))
    };

    /* no ice for size */
    static int *a1 = &s + sizeof(int [(int)(3.14 + 3.14)]);
    static int *a2 = sizeof(int [-(int)-3.14]) + &s;
    static int *a3 = &s - sizeof(int [!3.14 + 1]);
    static int *a4 = &s + sizeof(int [3.14 || 3.14]);
    static int *a5 = &s + sizeof(int [(1)? (int)(double)50: 0]);

    /* invalid */
    static int *b1 = &s + sizeof(int [(int)(3.14 + 3.14)]) + (int)(3.14 + 3.14);
    static int *b2 = sizeof(int [-(int)-3.14]) + &s - (int)-3.14;
    static int *b3 = &s - sizeof(int [!3.14 + 1]) - !3.14;
    static int *b4 = &s + sizeof(int [3.14 || 3.14]) + (3.14 || 3.14);
    static int *b5 = &s + sizeof(int [(1)? (int)(double)50: 0])+((1)? (int)(double)50: 0);
}

/* valid */
int *q1 = (((int)3.14)? 60: 0) + &s;
int *q2 = &s - ((1)? (int)70.0: 0);
int *q3 = &s + ((unsigned)3.14 / (unsigned)3.14);
int *q4 = ((int)3.14 == (int)3.14) + &s;

/* invalid */
int *q5 = &s - ((1)? (int)(double)50: 0);
