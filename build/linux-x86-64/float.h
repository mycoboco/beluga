/*
 *  float.h for x86-linux (IEEE 754)
 */

#ifndef _FLOAT
#define _FLOAT


typedef struct {
    int _Mmantdig;     /* MANT_DIG */
    int _Mdig;         /* DIG */
    int _Mminexp;      /* MIN_EXP */
    int _Mmin10exp;    /* MIN_10_EXP */
    int _Mmaxexp;      /* MAX_EXP */
    int _Mmax10exp;    /* MAX_10_EXP */
    union {
        unsigned char _Uc[12];    /* avoid sizeof */
        float _F;
        double _D;
        long double _Ld;
    } _Mmax,        /* MAX */
      _Mepsilon,    /* EPSILON */
      _Mmin;        /* MIN */
} _Fparam;


extern _Fparam _Dbl, _Flt, _Ldbl;


#define FLT_MANT_DIG    _Flt._Mmantdig
#define FLT_DIG         _Flt._Mdig
#define FLT_MIN_EXP     _Flt._Mminexp
#define FLT_MIN_10_EXP  _Flt._Mmin10exp
#define FLT_MAX_EXP     _Flt._Mmaxexp
#define FLT_MAX_10_EXP  _Flt._Mmax10exp
#define FLT_MAX         _Flt._Mmax._F
#define FLT_EPSILON     _Flt._Mepsilon._F
#define FLT_MIN         _Flt._Mmin._F

#define DBL_MANT_DIG    _Dbl._Mmantdig
#define DBL_DIG         _Dbl._Mdig
#define DBL_MIN_EXP     _Dbl._Mminexp
#define DBL_MIN_10_EXP  _Dbl._Mmin10exp
#define DBL_MAX_EXP     _Dbl._Mmaxexp
#define DBL_MAX_10_EXP  _Dbl._Mmax10exp
#define DBL_MAX         _Dbl._Mmax._D
#define DBL_EPSILON     _Dbl._Mepsilon._D
#define DBL_MIN         _Dbl._Mmin._D

#define LDBL_MANT_DIG   _Ldbl._Mmantdig
#define LDBL_DIG        _Ldbl._Mdig
#define LDBL_MIN_EXP    _Ldbl._Mminexp
#define LDBL_MIN_10_EXP _Ldbl._Mmin10exp
#define LDBL_MAX_EXP    _Ldbl._Mmaxexp
#define LDBL_MAX_10_EXP _Ldbl._Mmax10exp
#define LDBL_MAX        _Ldbl._Mmax._Ld
#define LDBL_EPSILON    _Ldbl._Mepsilon._Ld
#define LDBL_MIN        _Ldbl._Mmin._Ld

#define FLT_RADIX       2
#define FLT_ROUNDS      1    /* round to nearest */

#endif    /* _FLOAT */

/* end of float.h */
