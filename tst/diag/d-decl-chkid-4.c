/* -Wv --std=c90 */

int a234567891123456789212345678931a;
typedef int a234567891123456789212345678931a;    /* redecl */
typedef int a234567891123456789212345678931b;    /* conflict */
typedef int a234567891123456789212345678933a;

void f8(int a234567891123456789212345678932a) {
    typedef int a234567891123456789212345678932b,    /* conflict */
                a234567891123456789212345678931c,    /* conflict */
                a234567891123456789212345678934a;
    {
        typedef int a234567891123456789212345678931d;    /* conflict */
    }
    typedef int a234567891123456789212345678931d;    /* conflict */
}

typedef int a234567891123456789212345678934a;
