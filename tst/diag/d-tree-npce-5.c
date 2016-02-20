/* -Wv --std=c90 */

int (*p1)(void) = (1, 2, 0);

int (*p2)(void) = 0 && (1, 2, 0);
int (*p3)(void) = (1)? 0: (1, 2, 0);
int (*p4)(void) = 1 && (1, 2, 0);
int (*p5)(void) = (0)? 0: (1, 2, 0);

int (*p6)(void) = 0 && (1.0, 0);
int (*p7)(void) = (1)? 0: (1.0, 0);
int (*p8)(void) = 1 && (1.0, 0);
int (*p9)(void) = (0)? 0: (1.0, 0);

int (*p10)(void) = 0 && (p1, 0);
int (*p11)(void) = (1)? 0: (p1, 0);
