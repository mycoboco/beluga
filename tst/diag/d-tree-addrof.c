struct tag { int x; } f(void);
void f2(void) { struct tag y; int z; y = f(); z = f().x; }
void f3(void) { struct tag y; y = (f().x)? f(): y; }
void f4(void) { ((struct tag)12).x; }
