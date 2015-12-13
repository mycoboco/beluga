typedef struct { int m; } str;
typedef volatile struct { int m; } vstr;

void f(void)
{
    str Str;
    vstr Vstr;
    const str Cstr;

    (Str)? 1: 0;
    (Vstr)? 1: 0;
    (Cstr)? 1: 0;

    Str && 0;
    Vstr && 0;
    Cstr && 0;

    if (Str);
    if (Vstr);
    if (Cstr);

    while (Str);
    while (Vstr);
    while (Cstr);

    do; while (Str);
    do; while (Vstr);
    do; while (Cstr);

    for (;Str;);
    for (;Vstr;);
    for (;Cstr;);
}
