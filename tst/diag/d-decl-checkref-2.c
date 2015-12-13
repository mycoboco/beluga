extern void x;
extern const void cx;
extern volatile void vx;
extern const volatile void cvx;

extern void y;
extern const void cy;
extern volatile void vy;
extern const volatile void cvy;

void f(void)
{
    extern void x;                     /* no warning */
    extern const void cx;              /* no warning */
    extern volatile void vx;           /* no warning */
    extern const volatile void cvx;    /* no warning */

    extern void z;
    extern const void cz;
    extern volatile void vz;
    extern const volatile void cvz;

    x, cx, vx, cvx;
    y, cy, vy, cvy;
    z, cz, vz, cvz;
}
