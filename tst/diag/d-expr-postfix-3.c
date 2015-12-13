/* -Wv --plain-char=unsigned */

int a[10];

char c;
signed char sc;
int x = sizeof(a[c]),     /* warning */
    y = sizeof(a[sc]);

char f(void);
signed char sf(void);

int main(void)
{
    int r;
    int q = sizeof(a[c]),     /* warning */
        z = sizeof(a[sc]);

    r = a[c];                            /* warning */
    r = a[sc];
    r = a[f()];                          /* warning */
    r = a[sf()];
    r = a[(char)main()];                 /* warning */
    r = a[(signed char)main()];
    r = a[(int)(char)main()];
    r = a[(int)(signed char)main()];
    r = a[(int)c];
    r = a[(int)sc];
    r = a[(char)(int)(char)c];           /* warning */
    r = a[(signed char)(int)(char)c];

    r = a['0'];
    r = a[(char)'0'];
    r = a[(signed char)'0'];
    r = a[(char)1];
    r = a[(signed char)1];
    r = a[(char)-1];
    r = a[(signed char)-1];
    r = a[(char)255];
    r = a[(signed char)255];
    r = a[(char)(int)(char)255];
    r = a[(signed char)(int)(char)255];

    return r;
}
