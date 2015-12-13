void f1(void) { int i; i = (0+1) && i; }     /* fold */
void f2(void) { int i; i = (0u+1) && i; }    /* fold */
void f3(void) { int i; i = (1-1) && i; }     /* fold */
void f4(void) { int i; i = (1U-1) && i; }    /* fold */

void f6(void) { int i; i = (0+1) || i; }     /* fold */
void f7(void) { int i; i = (0U+1) || i; }    /* fold */
void f8(void) { int i; i = (1-1) || i; }     /* fold */
void f9(void) { int i; i = (1u-1) || i; }    /* fold */
