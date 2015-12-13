struct t1 { int x; }; struct t1 { int x; };       /* error */
struct t2 { int x; }; struct t2 { double y; };    /* error */
union t3  { int x; }; union t3  { int x; };       /* error */
struct t4 { int x; }; union t4  { int x; };       /* error */
struct t5 *x5_1; union t5 *x5_2;    /* error */
struct t6 *x6_1; struct t6 *x6_1;
enum t7 { X7_1  }; enum t7 { X7_2  };    /* error */
enum t8 { X8_1 };  enum t8 { Y10_2 };    /* error */
enum t9 { X9 }; struct t9 { int x; };    /* error */
struct t10 { int x; }; enum t10 { X10 };    /* error */
enum t11 x11; struct t11 *y11;    /* error */
struct t12 *y12; enum t12 x12;    /* error */
