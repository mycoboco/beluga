/* -Wv --std=c90 */

int a23456a;
int a23456a;
extern int a23456b;    /* conflict */
int a23456c;           /* conflict */

void f8(void)
{
    extern int a23457a;
}

int a23457b;    /* conflict */

static int a23456d;
static int a23456d891123456789212345678931a;
static int a23456d891123456789212345678931b;    /* conflict */
static int a234567891123456789212345678931b;
static int a234567891123456789212345678931c;    /* conflict */
