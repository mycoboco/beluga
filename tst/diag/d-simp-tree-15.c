void f1(void) { int i1; i1 = (3.141592F != 3.141592f); }     /* fold */
void f2(void) { int i2; i2 = (3.141592 != 3.1414591); }      /* fold */
void f3(void) { int i3; i3 = (3.141592L != 3.1415921l); }    /* fold */

void f5(void) { int i5; i5 = (42 != 42); }      /* fold */
void f6(void) { int i6; i6 = (42l != 43l); }    /* fold */
void f7(void) { struct t { int :2, x:4; } x; int i7; i7 = (x.x != 1); i7 = (x.x != 0); }           /* zerofield */
void f8(void) { struct t { unsigned :2, x:4; } x; int i8; i8 = (x.x != 1l); i8 = (x.x != 0l); }    /* zerofield */

void f10(void) { int i10; i10 = (42u != 42u); }      /* fold */
void f11(void) { int i11; i11 = (42UL != 43ul); }    /* fold */
void f12(void) { struct t { int :2, x:4; } x; int i12; i12 = (x.x != 1u); i12 = (x.x != 0u); }       /* zerofield */
void f13(void) { struct t { int :2, x:4; } x; int i13; i13 = (x.x != 1ul); i13 = (x.x != 0ul); }    /* zerofield */
