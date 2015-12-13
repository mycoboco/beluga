typedef struct tag str;
typedef volatile struct tag2 str2;
typedef int myint;
typedef myint aint1[];
typedef volatile myint aint2[];

void f(void)
{
    str *pstr;
    str2 *pstr2;
    aint1 *pa1;
    aint2 *pa2;

    pstr + 1;
    pstr2 + 1;
    pa1 + 1;
    pa2 + 1;

    pstr - pstr;
    pstr2 - pstr2;
    pa1 - pa1;
    pa2 - pa2;
}
