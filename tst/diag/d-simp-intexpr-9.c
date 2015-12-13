/* -Wv --std=c90 */

enum {
    A = (void *)0 == (void *)0,
    B = (void *)0 != (void *)0,
    C = (void *)0 >  (void *)0,
    D = (void *)0 >= (void *)0,
    E = (void *)0 <  (void *)0,
    F = (void *)0 <= (void *)0,
    G = (int)(void *)0,
    H = (void *)0 - (void *)0
};
