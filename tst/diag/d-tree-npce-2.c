extern int s;
extern double d;

/* invalid */
int *p1 = &s + (int)(void *)0;
int *p2 = &s - (int)(char *)1;
int *p3 = ((char *)1 - (char *)0) + &s;
int *p4 = ((char *)1 || (char *)0) + &s;
int *p5 = &s - ((char *)1 >= (char *)0);
int *p6 = (((void *)1)? 1: 0) + &s;
int *p7 = &s - (int)((1)? 0: (void *)0);
int *p8 = &s + ((unsigned)(void *)0 - (int)&d);
int *p9 = ((char *)1 || &d) + &s;
int *p10 = &s - ((char *)&d <= (char *)0);

void f(void)
{
    /* valid */
    int *a[] = {
        &s + sizeof((char *)1 - (char *)0),
        sizeof(!(char *)1) + &s,
        &s - sizeof((char *)1 && (char *)0),
        sizeof((char *)1 <= (char *)0) + &s,
        &s + sizeof((1)? (unsigned)(void *)70: 0)
    };

    int b[] = {
        sizeof(&s + (int)((char *)1 - (char *)0)),
        sizeof(!(char *)1 + &s),
        sizeof(&s - ((char *)1 && (char *)0)),
        sizeof(((char *)1 <= (char *)0) + &s),
        sizeof(&s + ((1)? (unsigned)(void *)50: 0))
    };

    /* no ice for size */
    static int *a1 = &s + sizeof(int [(int)(void *)0 + 1]);
    static int *a2 = sizeof(int [(int)(char *)1]) + &s;
    static int *a3 = &s - sizeof(int [!!(char *)1]);
    static int *a4 = &s + sizeof(int [(char *)1 || (char *)0]);
    static int *a5 = &s + sizeof(int [(1)? (int)(void *)70: 0]);

    /* invalid */
    static int *b1 = &s + sizeof(int [(int)(void *)0+1]) + (int)(void *)0+1;
    static int *b2 = sizeof(int [(int)(char *)1]) + &s - (int)(char *)1;
    static int *b3 = &s - sizeof(int [!!(char *)1]) - !!(char *)1;
    static int *b4 = &s + sizeof(int [(char *)1 || (char *)0]) + ((char *)1 || (char *)0);
    static int *b5 = &s + sizeof(int [(1)? (int)(void *)70: 0])+((1)? (int)(void *)70: 0);
}
