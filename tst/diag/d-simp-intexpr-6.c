/* -Wv --std=c99 */

int q;

int x1[(0 && 2)+1];
int x2[1 && 2];
int x3[1 || 2];
int x4[0 || 2];
int x5[(0 && 1.0)+1];            /* warning */
int x6[(0 && (int)1.0)+1];
int x7[(unsigned)(1 && 1.0)];    /* warning */
int x8[1 && (int)1.0];
int x9[1 || 1.0];                /* warning */
int x10[1 || (int)1.0];
int x11[(int)(0 || 1.0)];        /* warning */
int x12[0 || (unsigned)1.0];
int x13[(0 && &q)+1];            /* warning */
int x14[1 || &q];                /* warning */

int y1[(0, 1)];                          /* warning */
int y2[(0, 1, 2)];                       /* warning */
int y3[(0.0, 1, 2)];                     /* warning */
int y4[((int)0.0, 1, 2)];                /* warning */
int y5[(0, 1.0, 2)];                     /* warning */
int y6[(0, (int)1.0, 2)];                /* warning */
int y7[(0, (int)1.0, (unsigned)2.0)];    /* warning */

int z1[(0 && (1, 2))+1];
int z2[(0 && (1, 2, 3))+1];
int z3[(0 && (1.0, 2, 3))+1];              /* warning */
int z4[(0 && ((unsigned)1.0, 2, 3))+1];
int z5[(0 && (1, 2.0, 3))+1];              /* warning */
int z6[(0 && (1, (unsigned)2.0, 3))+1];
int z7[(0 && (1, 2, 3.0))+1];              /* warning */
int z8[(0 && (1, 2, (unsigned)3.0))+1];
int z9[(1, 2) && 1];                       /* warning */
int z10[(1, 2, 3) && 1];                   /* warning */
int z11[(1.0, 2, 3) && 1];                 /* warning */
int z12[((unsigned)1.0, 2, 3) && 1];       /* warning */
int z13[(1, 2.0, 3) && 1];                 /* warning */
int z14[(1, (unsigned)2.0, 3) && 1];       /* warning */

int w1[1 || (1, 2)];
int w2[1 || (1, 2, 3)];
int w3[1 || (1.0, 2, 3)];          /* warning */
int w4[1 || ((int)1.0, 2, 3)];
int w5[1 || (1, 2.0, 3)];          /* warning */
int w6[1 || (1, (int)2.0, 3)];
int w7[1 || (1, 2, 3.0)];          /* warning */
int w8[1 || (1, 2, (int)3.0)];
int w9[(1, 2) || 1];               /* warning */
int w10[(1, 2, 3) || 1];           /* warning */
int w11[(1.0, 2, 3) || 1];         /* warning */
int w12[((int)1.0, 2, 3) || 1];    /* warning */
int w13[(1, 2.0, 3) || 1];         /* warning */
int w14[(1, (int)2.0, 3) || 1];    /* warning */

int a1[(0, 1)? 1: 2];                      /* warning */
int a2[(0, 1, 2)? 1: 2];                   /* warning */
int a3[(0.0, 1, 2)? 1: 2];                 /* warning */
int a4[((int)0.0, 1, 2)? 1: 2];            /* warning */
int a5[(0, 1.0, 2)? 1: 2];                 /* warning */
int a6[(0, (unsigned)1.0, 2)? 1: 2];       /* warning */
int a7[(1)? (0, 1): 2];                    /* warning */
int a8[(1)? (0, 1, 2): 2];                 /* warning */
int a9[(1)? (0.0, 1, 2): 2];               /* warning */
int a10[(1)? ((int)0.0, 1, 2): 2];         /* warning */
int a11[(1)? 1: (0, 1)];
int a12[(1)? 1: (0, 1, 2)];
int a13[(1)? 1: (0.0, 1, 2)];              /* warning */
int a14[(1)? 1: ((unsigned)0.0, 1, 2)];
int a15[(1)? 1: (0, 1.0, 2)];              /* warning */
int a16[(1)? 1: (0, (int)1.0, 2)];
