enum;                                     /* error */
enum { };    /* error */
enum e3;                                  /* error */
enum e4 x; enum e4 z; enum e4 { E10 };    /* error */
enum e5 { E1 }; enum e5;    /* error */
enum e6 { E1 };            /* error */
enum e7 { E2 }; enum e7 *y;
enum e8 { E3, E3 };        /* error */
enum e9 { E4, E5, E4 };    /* error */
enum e10 { E6 = 0x7ffffffe, E7 };
enum e11 { E8 = 0x7fffffff, E9, E9_2 };    /* error */
enum e12 { int x; };    /* error */
enum e13 { E6, E7, };    /* error */
enum e14 { E14_1, E14_2; };    /* error */
enum e15 { E15 = 0x80000000+1, E16 = 0xffffffff };    /* error */
