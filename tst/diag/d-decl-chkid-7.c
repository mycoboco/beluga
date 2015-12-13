/* -Wv --std=c90 */

extern int a23456();

void f3(void)
{
    extern void a23457();
    a23456();
    a23456a();    /* conflict */
    a23458();
}

void f4(void)
{
    a23457a();    /* conflict */
}
