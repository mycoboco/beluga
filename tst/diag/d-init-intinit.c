char a1[] = { { 1 }, { 2 }, { 3 }, 4 };
char a2[] = { { { 1 }, { 2 }, }, { 3 }, { { 4 } } };    /* warning */
char a3[] = { 1, a1 };                                   /* error */
char a4[] = { 1, a2[0] };                                /* error */
struct t { const int x:32, :6, y:4; };
struct t t6 = { -2147483647-1, -9 };                     /* warning */
struct t t7 = { 2147483647, 8 };                         /* warning */
struct s { unsigned x:32, :6, y:4; };
struct s t9 = { { { 4294967295 } }, 16 };                /* warning */
struct s t10 = { { -2147483647-1, }, -16 };
