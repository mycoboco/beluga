/* -Wv */

# 1 "d-33-1-a.c"
void f(void)
{
    int a;

    if (a);

# 1 "d-33-1-b.c" 1
    int b;
    if (b);

# 1 "d-33-1-c.c" 1
    int c;
    if (c);
# 5 "d-33-1-b.c" 2

    if (b);
# 8 "d-33-1-a.c" 2

    if (a);
}
