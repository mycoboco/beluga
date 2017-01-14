/* -Wv --std=c90 */

#define VOID void

int x = (int *foo)0;

void ff(const VOID, ...);
void gg(...);
void hh(const void xx);
void ii(void xx);
void jj(int, ..., double xx);

void kk(void **((pa))()) {}

void kj(int, ++);
void mm(int --xx);
void nn(int **++xx);

void zz(= 1 + 0);
void wz(id, int, ...);
