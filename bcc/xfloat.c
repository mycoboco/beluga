/*
 *  float.h for x86-linux (IEEE 754)
 */

#include "float.h"

/* significand */
#define SIGF 24
#define SIGD 53
#define SIGL 64

/* max exponent */
#define ONES(n) ((1 << (n)) - 1)    /* should be signed */
#define MAXEF   ONES(32-1-(SIGF-1))
#define MAXED   ONES(64-1-(SIGD-1))
#define MAXEL   ONES(80-1-SIGL)

/* exponent bias */
#define BIASF (MAXEF >> 1)
#define BIASD (MAXED >> 1)
#define BIASL (MAXEL >> 1)

#define LOG2 0.30103

/* bbbbbbbb ffffffff efffffff 0eeeeeee */
#define INIT4(e, f, b) (b), (f), ((f)>>1) | ((e)&0x01U)<<7, (e)>>1

/* bbbbbbbb ffffffff ffffffff ffffffff  ffffffff ffffffff eeeeffff 0eeeeeee */
#define INIT8(e, f, b) (b), (f), (f), (f), (f), (f), (f)>>4 | ((e)&0x0FU)<<4, (e)>>4

/* bbbbbbbb ffffffff ffffffff ffffffff  f...f 1fffffff  eeeeeeee 0eeeeeee 00000000 00000000 */
#define INIT12(e, f, b) (b), (f), (f), (f), (f), (f), (f), 1U<<7 | (f)>>1, (e)&0xFFU, (e)>>8


_Fparam _Flt = {
    (int)SIGF,                                /* FLT_MANT_DIG */
    (int)((SIGF-1) * LOG2),                   /* FLT_DIG */
    (int)(1-BIASF+1),                         /* FLT_MIN_EXP */
    (int)((-BIASF+1) * LOG2),                 /* FLT_MIN_10_EXP */
    (int)(BIASF+1),                           /* FLT_MAX_EXP */
    (int)((BIASF+1) * LOG2),                  /* FLT_MAX_10_EXP */
    { INIT4(MAXEF-1,        0xFF, 0xFF) },    /* FLT_MAX */
    { INIT4(BIASF-(SIGF-1), 0x00, 0x00) },    /* FLT_EPSILON */
    { INIT4(0x01,           0x00, 0x00) },    /* FLT_MIN */
};

_Fparam _Dbl = {
    (int)SIGD,                                /* DBL_MANT_DIG */
    (int)((SIGD-1) * LOG2),                   /* DBL_DIG */
    (int)(1-BIASD+1),                         /* DBL_MIN_EXP */
    (int)((-BIASD+1) * LOG2),                 /* DBL_MIN_10_EXP */
    (int)(BIASD+1),                           /* DBL_MAX_EXP */
    (int)((BIASD+1) * LOG2),                  /* DBL_MAX_10_EXP */
    { INIT8(MAXED-1,        0xFF, 0xFF) },    /* DBL_MAX */
    { INIT8(BIASD-(SIGD-1), 0x00, 0x00) },    /* DBL_EPSILON */
    { INIT8(0x001,          0x00, 0x00) },    /* DBL_MIN */
};

_Fparam _Ldbl = {
    (int)SIGL,                                 /* LDBL_MANT_DIG */
    (int)((SIGL-1) * LOG2),                    /* LDBL_DIG */
    (int)(1-BIASL+1),                          /* LDBL_MIN_EXP */
    (int)((-BIASL+1) * LOG2),                  /* LDBL_MIN_10_EXP */
    (int)(BIASL+1),                            /* LDBL_MAX_EXP */
    (int)((BIASL+1) * LOG2),                   /* LDBL_MAX_10_EXP */
    { INIT12(MAXEL-1,        0xFF, 0xFF) },    /* LDBL_MAX */
    { INIT12(BIASL-(SIGL-1), 0x00, 0x00) },    /* LDBL_EPSILON */
    { INIT12(0x0001,         0x00, 0x00) },    /* LDBL_MIN */
};

/* end of xfloat.c */
