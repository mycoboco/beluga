struct t x1 = { 0, { 1, 2, 3 }, };                   /* error */
union u { double x; } u2 = 0;                        /* error */
union u u3 = { 0, };
union u u41 = { 0, 1, }, u42 = { 0, { 0, 1, }, };    /* error */
struct t { double x; int y; void *z; };
struct t x6 = { 3.14, 0, &x6 };
struct t x7 = { 3.14, { 0, }, &x6 };
struct t x8 = 3.14;                                  /* error */
