/* -Wv --logical-shift */

void f(void)
{
    int i;

    i = i / ((-1 >> 2) + 1);
    i = i / (1 >> 2);
}
