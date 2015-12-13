/* -v */

f1(void) {}
int f2(void) {}         /* warning */
signed f3(void) {}      /* warning */
unsigned f4(void) {}    /* warning */
double f5(void) {}      /* warning */
*f6(void) {}            /* warning */
const f7(void) {}       /* warning */

g1() {}
int g2() {}         /* warning */
signed g3() {}      /* warning */
unsigned g4() {}    /* warning */
double g5() {}      /* warning */
*g6() {}            /* warning */
const g7() {}       /* warning */
