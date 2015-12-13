void f1(void) { int i1; i1 = (3.141592F <= 3.141592f); }     /* fold */
void f2(void) { int i2; i2 = (3.141592 <= 3.1414591); }      /* fold */
void f3(void) { int i3; i3 = (3.141592l <= 3.1415921L); }    /* fold */

void f5(void) { int i5; i5 = (42 <= 42); }      /* fold */
void f6(void) { int i6; i6 = (42l <= 43L); }    /* fold */

void f8(void)  { int i8; i8 = (42u <= 42u); }      /* fold */
void f9(void)  { int i9; i9 = (42ul <= 43UL); }    /* fold */
void f10(void) { unsigned u;      int i10; i10 = (0 <= u); }    /* geu */
void f11(void) { unsigned long m; int i11; i11 = (0 <= m); }    /* geu */
void f12(void) { unsigned u;      int i12; i12 = (u <= 0); }    /* to == */
void f13(void) { unsigned long m; int i13; i13 = (m <= 0); }    /* to == */
