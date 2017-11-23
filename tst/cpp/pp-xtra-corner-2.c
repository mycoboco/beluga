
/* from WG21's DR1625 */

#define F(A, B, C) A ## x.B ## y.C ## z
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)

char v[] = EXPAND_AND_STRINGIFY(F(a, b, c))
