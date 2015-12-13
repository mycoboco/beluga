typedef int myint;
typedef volatile myint vint;
typedef struct { int m; } str;
typedef volatile struct { int m; } vstr;

vint f(void)
{
    str Str;
    return Str;
}

vint f2(void)
{
    vstr Vstr;
    return Vstr;
}
