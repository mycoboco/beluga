struct tag { m1 };    /* error */
union { m2 };         /* error */
struct;    /* error */
struct m4;             struct m4;               /* okay, same */
struct m5 { int x; };  struct m5;               /* okay, same */
struct m6 *x;          struct m6;               /* okay, same */
struct m7;             struct m7 { int x; };    /* okay, same */
struct m8 { int x; };  struct m8 { int x; };    /* error */
struct m9 *y;          struct m9 { int x; };    /* okay, same */
struct m10;            struct m10 *z;           /* okay, same */
struct m11 { int x; }; struct m11 o;            /* okay, same */
struct m12 *p;         struct m12 *q;           /* okay, same */
struct m13;            union  m13;               /* error */
union  m14 { int x; }; struct m14;               /* error */
struct m15 *r;         union  m15;               /* error */
union  m16;            struct m16 { int x; };    /* error */
struct m17 { int x; }; union  m17 { int x; };    /* error */
union  m18 *s;         struct m18 { int x; };    /* error */
struct m19;            union  m19 *t;            /* error */
union  m20 { int x; }; struct m20 *u;            /* error */
struct m21 *v;         union  m21 *w;            /* error */
void f22(void) { struct m22;            struct m22 *x; { struct m22; struct m22 *a; x = a; } }    /* diff */
void f23(void) { struct m23 { int x; }; struct m23 *x; { struct m23; struct m23 *a; x = a; } }    /* diff */
void f24(void) { struct m24 *x;                        { struct m24; struct m24 *a; x = a; } }    /* diff */
void f25(void) { struct m25;            struct m25 *x; { struct m25 { int x; } *a;  x = a; } }    /* diff */
void f26(void) { struct m26 { int x; }; struct m26 *x; { struct m26 { int x; } *a;  x = a; } }    /* diff */
void f27(void) { struct m27 *x;                        { struct m27 { int x; } *a;  x = a; } }    /* diff */
void f28(void) { struct m28;            struct m28 *x; { struct m28 *a;             x = a; } }    /* same */
void f29(void) { struct m29 { int x; }; struct m29 *x; { struct m29 *a;             x = a; } }    /* same */
void f30(void) { struct m30 *x;                        { struct m30 *a;             x = a; } }    /* same */
void f31(void) { struct m31;            struct m31 *x; { union  m31; union  m31 *a; x = a; } }    /* diff */
void f32(void) { union  m32 { int x; }; union  m32 *x; { struct m32; struct m32 *a; x = a; } }    /* diff */
void f33(void) { struct m33 *x;                        { union  m33; union  m33 *a; x = a; } }    /* diff */
void f34(void) { union  m34;            union  m34 *x; { struct m34 { int x; } *a;  x = a; } }    /* diff */
void f35(void) { struct m35 { int x; }; struct m35 *x; { union  m35 { int x; } *a;  x = a; } }    /* diff */
void f36(void) { union  m36 *x;                        { struct m36 { int x; } *a;  x = a; } }    /* diff */
void f37(void) { struct m37;            struct m37 *x; { union  m37 *a;             x = a; } }    /* error */
void f38(void) { union  m38 { int x; }; union  m38 *x; { struct m38 *a;             x = a; } }    /* error */
void f39(void) { struct m39 *x;                        { union  m39 *a;             x = a; } }    /* error */
struct m40 { };    /* error */
struct { };        /* error */
