/* -Wv --std=c99 */

struct t { int m; int *p;
           signed int a:10; unsigned b:10; } s;

int i1 = (s, 1);
int i2 = 1 || (s, 1);

int i3 = ((void)1, 1);
int i4 = 0 && ((void)1, 1);

struct t s1 = { (int *)0-(int *)0,
                &i1+(int)(3.14+3),
                1 || ("abc", 1),
                0 && ((void *)0, 1) };

int a1[] = { 0, (int *)0-(int *)0, 3.14,
                (1)? 1: ((void)1, 1) };
int *a2[] = { &i1+(int)(3.14+3),
              (int *)(int)(void *)0 };
char a3[] = { (1)? 1: ((void *)0, 1) };
