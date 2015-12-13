double x11 = 3, x12 = { 3, };
double x21 = { { 3, }, };         /* warning */
double x31 = { { { 3, } };        /* error */
int i41 = { 0, 1 };               /* error */
int i5 = (void *)0;               /* error */
extern struct t x; int i6 = x;    /* error */
