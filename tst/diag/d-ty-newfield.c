struct t1 { int m1, m1; };           /* error */
struct t2 { int m1; double m1; };    /* error */
struct t3 { int m1; union { int m1; } x; };
struct t4 { int m1; union { int m1; } x; int m1; };    /* error */
struct t5 { int (*m1)(void); int ((((m1)))); };    /* error */
