/* -Wv --std=c90 */

int a23456;
static int a234567891123456789212345678934a;
typedef int a234567891123456789212345678936a;
enum { a234567891123456789212345678937 };

void f8(void)
{
    extern int a23457;
    extern int a23458;
}

void f13(int a234567891123456789212345678935)
{
    int a234567891123456789212345678931;
    int a234567891123456789212345678931a;             /* conflict */
    static int a234567891123456789212345678931b;      /* conflict */
    register int a234567891123456789212345678931c;    /* conflict */
    static int a234567891123456789212345678932;

    extern int a23456a;                             /* conflict */
    extern int a234567891123456789212345678934b;    /* conflict */

    extern int a23457;
    extern int a23457a;    /* conflict */
    extern int a23458a;    /* conflict */
}
