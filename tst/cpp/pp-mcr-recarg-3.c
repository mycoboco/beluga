#define FOO1(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31, p32, p33)
FOO1(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32)    /* warning, insufficient */

#define FOO2(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31)
FOO2(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31)

#define FOO3(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31, p32, p33)
FOO3(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32, a33)    /* warning */

#define FOO4(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31, p32)
FOO4(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32)    /* warning */

#define FOO5(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31, p32)
FOO5(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32, a33)    /* warning, error */

#define FOO6(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30)
FOO6(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32)    /* error */

#define FOO7(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, \
             p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, \
             p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, \
             p31)
FOO7(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
     a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
     a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
     a31, a32)    /* error */

#define MCR(x) FOO##x(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, \
                      a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, \
                      a21, a22, a23, a24, a25, a26, a27, a28, a29, a30 a30, \
                      a31, a32, a33)    /* warning, error */
/* ... */ MCR(5)
