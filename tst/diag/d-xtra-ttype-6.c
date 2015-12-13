typedef int int1;
typedef int *ptoint;
typedef const int cint;
typedef const int1 *ptocint;
typedef float fp;
typedef volatile float vfp;
typedef struct { int x; } str;
typedef volatile struct { int x; } vstr;
typedef struct inc inc;

void f1(void)
{
    ptoint x;
    void f2(unsigned *);
    int1 a[10];
    const int1 a2[10];
    cint a3[10];
    ptocint y;
    fp f;
    vfp vf;
    str s;
    vstr vs;
    const str cs;
    extern inc is;
    inc *pis;

    f2(x);
    f2(*y);

    a[a];
    a2[a2];
    a3[a3];
    (*y)[0];
    x();
    a();
    a3();
    (*y)();
    x.m;
    a.m;
    a3.m;
    (*y).m;
    x->m;
    a->m;
    a3->m;
    (*y)->m;
    a++;
    a2++;
    a3++;
    (&*y)++;
    s++;
    vs++;
    cs++;

    ++a;
    ++a2;
    ++s;
    ++vs;
    ++cs;
    (&x).m;
    (&a).m;
    (&a2).m;
    (&a3).m;
    (&y).m;
    (&vs).m;
    (&cs).m;
    *f;
    *vf;
    *s;
    *vs;
    *cs;
    +x;
    +a;
    +a2;
    +a3;
    +y;
    +s;
    +vs;
    +cs;
    -x;
    -a;
    -a2;
    -a3;
    -y;
    -s;
    -vs;
    -cs;
    ~x;
    ~a;
    ~a2;
    ~a3;
    ~y;
    ~s;
    ~vs;
    ~cs;
    !s;
    !vs;
    !cs;
    sizeof(is);
    sizeof(*pis);

    x * x;
    a * a;
    a2 * a2;
    a3 * a3;
    y * y;
    s * s;
    vs * vs;
    cs * cs;

    x + x;
    a + a;
    a2 + a2;
    a3 + a3;
    y + y;
    s + s;
    vs + vs;
    cs + cs;

    x - s;
    a - s;
    a2 - s;
    a3 - s;
    y - s;
    s - s;
    vs - vs;
    cs - cs;

    x << s;
    a << s;
    a2 << s;
    a3 << s;
    y << s;
    s << s;
    vs << vs;
    cs << cs;

    x < s;
    a < s;
    a2 < s;
    a3 < s;
    y < s;
    s < s;
    vs < vs;
    cs < cs;

    x == s;
    a == s;
    a2 == s;
    a3 == s;
    y == s;
    s == s;
    vs == vs;
    cs == cs;

    x | s;
    a | s;
    a2 | s;
    a3 | s;
    y | s;
    s | s;
    vs | vs;
    cs | cs;

    s && s;
    vs && vs;
    cs && cs;

    (1)? x: s;
    (1)? a: s;
    (1)? a2: s;
    (1)? a3: vs;
    (1)? y: cs;

    x = s;
    a = s;
    a2 = s;
    a3 = vs;
    y = cs;
}
