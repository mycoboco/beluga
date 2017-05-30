typedef struct { int x; } ux_t;

#define SYM(p) (~(~(ux_t)0 << (p)))
