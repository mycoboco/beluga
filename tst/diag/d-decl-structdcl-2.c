/* --std=c90 -v */
struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x1;
struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x2;    /* warning */
void f3(struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x);
void f4(struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x);    /* warning */
void f5(void) { int x = sizeof(struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; }); }
void f6(void) { int x = sizeof(struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; }); }    /* warning */
void f7(void) {
    struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x1;
    struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x2;    /* warning */
&x1, &x2; }
void f8(struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } p1,
struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } p2);    /* warning */
struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { struct { int x; struct { struct { int x; } x; } x; struct { struct { int x; } x; } y; } z; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x; } x;    /* warning */
