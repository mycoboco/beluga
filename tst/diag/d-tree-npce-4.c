/* -Wv --std=c90 */

extern int s;
extern double d;

/* valid */
int *p1 = &s + (0 && 2)+1;
int *p2 = &s - (1 && 2);
int *p3 = (1 || 2) + &s;
int *p4 = &s - (0 || 2);
int *p5 = (0 && (unsigned)1.0) + &s;

/* invalid */
int *q1 = (0 && 1.0) + &s;
int *q2 = &s + (0 && &d);
int *q3 = (0.0, 1, 2) + &s;
int *q4 = (0 && (0.0, 1, 2)) + &s;
int *q5 = &s + (0 && (0, 1.0, 2));
int *q6 = (1 || (0.0, 1, 2)) + &s;
int *q7 = &s - (1 || (0, 1, 2.0));
int *q8 = &s - ((0.0, 1)? 1: 2);
int *q9 = ((1)? (0.0, 1): 2) + &s;
int *q10 = ((1)? 1: (0, 1.0, 2)) + &s;
int *q11 = (0, 1) + &s;
int *q12 = &s - (0, 1, 2);
int *q13 = ((int)0.0, 1, 2) + &s;
int *q14 = &s - ((0, 1)? 1: 2);
int *q15 = ((1)? (0, 1): 2) + &s;
int *q16 = &s - (((int)0.0, 1)? 1: 2);
int *q17 = ((1)? ((unsigned)0.0, 1): 2) + &s;
int *q18 = (1 || (0, 1, (int)2.0)) + &s;
int *q19 = ((1)? 1: (0, 1, 2)) + &s;
int *q20 = ((1)? 1: (0, (int)1.0, 2)) + &s;
int *q21 = (0 && ((unsigned)0.0, 1, 2)) + &s;

void f(void)
{
    /* valid */
    int *a[] = {
        &s + sizeof(0 && 1.0),
        sizeof(0 && &d) + &s,
        &s - sizeof(0.0, 1, 2),
        sizeof(0 && (0.0, 1, 2)) + &s,
        &s + sizeof(1 || (0, 1, 2.0)),
        sizeof((1)? (0.0, 1): 2) + &s,
        sizeof((1)? 1: (0, 1.0, 2)) + &s
    };

    int b[] = {
        sizeof(0 && 1.0),
        sizeof(0 && &d),
        sizeof(0.0, 1, 2),
        sizeof(0 && (0.0, 1, 2)),
        sizeof(1 || (0, 1, 2.0)),
        sizeof((1)? (0.0, 1): 2),
        sizeof((1)? 1: (0, 1.0, 2))
    };

    /* invalid */
    static int *b1 = &s + sizeof(int [(int)(void *)0+1]) + (int)(void *)0+1;
    static int *b2 = sizeof(int [(int)(char *)1]) + &s - (int)(char *)1;
    static int *b3 = &s - sizeof(int [!!(char *)1]) - !!(char *)1;
    static int *b4 = &s + sizeof(int [(char *)1 || (char *)0]) + ((char *)1 || (char *)0);
    static int *b5 = &s + sizeof(int [(1)? (int)(void *)70: 0])+((1)? (int)(void *)70: 0);

    static int *c1 = &s + sizeof(0 && 1.0) + (0 && 1.0);
    static int *c2 = sizeof(0 && &d) + &s + (0 && &d);
    static int *c3 = &s - sizeof(0.0, 1, 2) - (0.0, 1, 2);
    static int *c4 = sizeof(0 && (0.0, 1, 2)) + &s + (0.0, 1, 2);
    static int *c5 = &s + sizeof(1 || (0, 1, 2.0)) + (1 || (0, 1, 2.0));
    static int *c6 = sizeof((1)? (0.0, 1): 2) + &s + ((1)? (0.0, 1): 2);
    static int *c7 = sizeof((1)? 1: (0, 1.0, 2)) + &s + ((1)? 1: (0, 1.0, 2));
}
