void f1(void) { int i1; i1 = (3.141592f >= 3.141592F); }     /* fold */
void f2(void) { int i2; i2 = (3.141592 >= 3.1414591); }      /* fold */
void f3(void) { int i3; i3 = (3.141592L >= 3.1415921l); }    /* fold */

void f5(void) { int i5; i5 = (42 >= 42); }      /* fold */
void f6(void) { int i6; i6 = (42l >= 43l); }    /* fold */

void f8(void)  { int i8; i8 = (42u >= 42U); }      /* fold */
void f9(void)  { int i9; i9 = (42ul >= 43Ul); }    /* fold */
void f10(void) { unsigned u;      int i10; i10 = (u >= 0); }    /* geu */
void f11(void) { unsigned long m; int i11; i11 = (m >= 0); }    /* geu */
void f12(void) { unsigned u;      int i12; i12 = (0 >= u); }    /* to == */
void f13(void) { unsigned long m; int i13; i13 = (0 >= m); }    /* to == */
