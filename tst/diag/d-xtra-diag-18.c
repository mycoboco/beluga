/* -Wv --std=c90 */

int ((xx)) = { { { 0, } } };
int yy[1] = { 1, 100, 101,, ??! };
char cc[1] = { 1, 100 + 100 };
char dd[1] = { 1, 100 + 100, 'foo',, };
struct { int m; } zz = { 1, 100 };
struct { int m; } vv = { 1, 100*100,, };
struct { int m; } ww = { 1, return, if };
struct {} qq = { 0, 100 };
int ee[] = 1000+1;
struct { int m; } ss = 100 + 100;
union { int m; } uu = 100 + 100;
