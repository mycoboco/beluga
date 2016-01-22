/* -Wv */

struct x { int (*m1)();
           char m2:2;              /* error */
           unsigned short m3:8;    /* error */
           int m4:4;
           unsigned int m5:4;
           signed int m6:4;
           unsigned int m7:x;      /* error */
           unsigned int m8:-1;     /* error */
           unsigned int m9:32;
           unsigned int m10:33;    /* error */
           int m11:0;    /* error */
           struct m12 { int x; };
           struct { int m13; int m16; };
           struct x m14;    /* error */
           int m15(); };    /* error */
