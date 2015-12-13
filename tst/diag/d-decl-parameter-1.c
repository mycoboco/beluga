void f1(char c, ..., ..., ...) { }    /* error */
void f2(const void) { }                   /* error */
void f3(void id) { }                      /* error */
void f4(const void id) { }                /* error */
void f5(void, const void) { }             /* error */
void f6(void, const void id) { }          /* error */
void f7(const void, void) { }             /* error */
void f8(const void, const void id) { }    /* error */
void f9(void, ...) { }                    /* error */
void f10(const void, ...) { }             /* error */
void f11(void id, ...) { }                /* error */
void f12(const void id, ...) { }          /* error */
void f13(...) { }                                /* error */
void f14(char c, ..., char d) { }                /* error */
void f15(int a, ..., int b, ...) { }             /* error */
void f16(int a, ..., int b, ..., void) { }       /* error */
void f17(int a, ..., int b, ..., void id) { }    /* error */
