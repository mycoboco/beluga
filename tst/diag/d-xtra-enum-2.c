typedef enum e1 { A } e1;
typedef enum e2 { B } e2;
typedef struct { int m; } str;
typedef float _flt;
typedef double _dbl;
typedef long double _ld;
typedef signed char _sc;
typedef unsigned char _uc;
typedef short _shrt;
typedef int _i;
typedef unsigned _u;
typedef long _l;
typedef unsigned long _ul;
typedef void *_vp;

str st;

/* binary */
void f1(void)
{
    e1 x;
    e2 y;

    (x + y) = st;
}

/* check overflow */
void f2(void)
{
    _i x1 = 2147483648.0;
    e1 x2 = 2147483648.0;
    _i x3 = -2147483649.0l;
    e2 x4 = -2147483649.0l;
    _i x5 = 2147483648.0f;
    e2 x6 = 2147483648.0f;
    e1 x7 = 0x80000000;
    _sc x8 = (enum e1)128;
    _shrt x9 = (enum e2)0x8000;
}

/* conversion */
void f3(void)
{
    _flt f;
    _dbl d;
    _ld ld;
    _uc uc;
    _shrt s;
    _u u;
    _l l;
    _ul ul;
    _vp *p;
    e1 e;

    ((e1)f) = st;
    ((e2)d) = st;
    ((e1)ld) = st;
    ((e2)uc) = st;
    ((e1)s) = st;
    ((e2)u) = st;
    ((e1)l) = st;
    ((e2)ul) = st;
    ((e1)p) = st;

    ((_flt)e) = st;
    ((_dbl)e) = st;
    ((_ld)e) = st;
    ((_uc)e) = st;
    ((_shrt)e) = st;
    ((_u)e) = st;
    ((_l)e) = st;
    ((_ul)e) = st;
    ((_vp)e) = st;
}

/* derefernce */
void f4(void)
{
    e1 *p1;
    const e1 *p2;

    *p1 = st;
    *p2 = st;
}

/* assignment to bitfield */
void f5(void)
{
    struct {
        e1 x: 1;
        signed int a: 2;
        unsigned int b: 2;
    } s;

    s.a = (e1)0;
    s.a = (e1)1;
    s.a = (e1)3;

    (s.a = (e1)0) = st;
    (s.b = (e1)1) = st;
    (s.b = (e1)3) = st;
}
