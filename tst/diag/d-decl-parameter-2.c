void f1(int id, char id) { }          /* error */
void f2(int id, int id) { }           /* error */
void f3(int id) { int id; }           /* error */
void f4(int id) { double id; }        /* error */
void f5(int id) { extern int id; }    /* error */
