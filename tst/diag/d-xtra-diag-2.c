/* -Wv --std=c90 */

int foo;
enum { foo };
int fred;

enum {
    bar = fred >> 2,
    barbar = (int)(3.14 - 3.14),
    foobar = 0x7fffffff + 1,
    max = 0x7fffffff,
    over,
}

enum x;

enum *p;

enum a234567891123456789212345678931234 { A };
union a234567891123456789212345678931235 { int x; };
