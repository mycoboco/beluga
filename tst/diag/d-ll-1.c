/* -Wv */

void f(void)
{
    (1)? 1: (void *)0;
    (1)? 2147483647: (void *)0;
    (1)? 2147483648: (void *)0;
    (1)? 4294967295: (void *)0;
    (1)? 4294967296: (void *)0;
    (1)? 9223372036854775807: (void *)0;
    (1)? 9223372036854775808: (void *)0;
    (1)? 18446744073709551615: (void *)0;
    (1)? 18446744073709551616: (void *)0;

    (1)? 1U: (void *)0;
    (1)? 2147483647u: (void *)0;
    (1)? 2147483648U: (void *)0;
    (1)? 4294967295u: (void *)0;
    (1)? 4294967296U: (void *)0;
    (1)? 9223372036854775807u: (void *)0;
    (1)? 9223372036854775808U: (void *)0;
    (1)? 18446744073709551615u: (void *)0;
    (1)? 18446744073709551616U: (void *)0;

    (1)? 1L: (void *)0;
    (1)? 2147483647l: (void *)0;
    (1)? 2147483648L: (void *)0;
    (1)? 4294967295l: (void *)0;
    (1)? 4294967296L: (void *)0;
    (1)? 9223372036854775807l: (void *)0;
    (1)? 9223372036854775808L: (void *)0;
    (1)? 18446744073709551615l: (void *)0;
    (1)? 18446744073709551616L: (void *)0;

    (1)? 1Ul: (void *)0;
    (1)? 2147483647Lu: (void *)0;
    (1)? 2147483648lU: (void *)0;
    (1)? 4294967295uL: (void *)0;
    (1)? 4294967296UL: (void *)0;
    (1)? 9223372036854775807lu: (void *)0;
    (1)? 9223372036854775808lU: (void *)0;
    (1)? 18446744073709551615Lu: (void *)0;
    (1)? 18446744073709551616LU: (void *)0;

    (1)? 1LL: (void *)0;
    (1)? 2147483647ll: (void *)0;
    (1)? 2147483648LL: (void *)0;
    (1)? 4294967295ll: (void *)0;
    (1)? 4294967296LL: (void *)0;
    (1)? 9223372036854775807ll: (void *)0;
    (1)? 9223372036854775808LL: (void *)0;
    (1)? 18446744073709551615ll: (void *)0;
    (1)? 18446744073709551616LL: (void *)0;

    1234567lL;
    1234567Ll;

    (1)? 1uLL: (void *)0;
    (1)? 2147483647llU: (void *)0;
    (1)? 2147483648LLu: (void *)0;
    (1)? 4294967295llU: (void *)0;
    (1)? 4294967296ULL: (void *)0;
    (1)? 9223372036854775807Ull: (void *)0;
    (1)? 9223372036854775808uLL: (void *)0;
    (1)? 18446744073709551615Ull: (void *)0;
    (1)? 18446744073709551616LLU: (void *)0;

    1234567uLl;
    1234567lul;
    1234567LUL;
    1234567lLU;
}

void g(void)
{
    (1)? 01: (void *)0;
    (1)? 0x7fffffff: (void *)0;
    (1)? 0x80000000: (void *)0;
    (1)? 0xffffffff: (void *)0;
    (1)? 0x100000000: (void *)0;
    (1)? 0x7fffffffffffffff: (void *)0;
    (1)? 0x8000000000000000: (void *)0;
    (1)? 0xffffffffffffffff: (void *)0;
    (1)? 0x10000000000000000: (void *)0;

    (1)? 0x1U: (void *)0;
    (1)? 0x7fffffffu: (void *)0;
    (1)? 0x80000000U: (void *)0;
    (1)? 0xffffffffu: (void *)0;
    (1)? 0x100000000U: (void *)0;
    (1)? 0x7fffffffffffffffu: (void *)0;
    (1)? 0x8000000000000000U: (void *)0;
    (1)? 0xffffffffffffffffu: (void *)0;
    (1)? 0x10000000000000000U: (void *)0;

    (1)? 01L: (void *)0;
    (1)? 0x7fffffffl: (void *)0;
    (1)? 0x80000000L: (void *)0;
    (1)? 0xffffffffl: (void *)0;
    (1)? 0x100000000L: (void *)0;
    (1)? 0x7fffffffffffffffl: (void *)0;
    (1)? 0x8000000000000000L: (void *)0;
    (1)? 0xffffffffffffffffl: (void *)0;
    (1)? 0x10000000000000000L: (void *)0;

    (1)? 0x1Ul: (void *)0;
    (1)? 0x7fffffffLu: (void *)0;
    (1)? 0x80000000lU: (void *)0;
    (1)? 0xffffffffuL: (void *)0;
    (1)? 0x100000000UL: (void *)0;
    (1)? 0x7ffffffffffffffflu: (void *)0;
    (1)? 0x8000000000000000lU: (void *)0;
    (1)? 0xffffffffffffffffLu: (void *)0;
    (1)? 0x10000000000000000LU: (void *)0;

    (1)? 0x1LL: (void *)0;
    (1)? 0x7fffffffll: (void *)0;
    (1)? 0x80000000LL: (void *)0;
    (1)? 0xffffffffll: (void *)0;
    (1)? 0x100000000LL: (void *)0;
    (1)? 0x7fffffffffffffffll: (void *)0;
    (1)? 0x8000000000000000LL: (void *)0;
    (1)? 0xffffffffffffffffll: (void *)0;
    (1)? 0x10000000000000000LL: (void *)0;

    (1)? 0x1uLL: (void *)0;
    (1)? 0x7fffffffllU: (void *)0;
    (1)? 0x80000000LLu: (void *)0;
    (1)? 0xffffffffllU: (void *)0;
    (1)? 0x100000000ULL: (void *)0;
    (1)? 0x7fffffffffffffffUll: (void *)0;
    (1)? 0x8000000000000000uLL: (void *)0;
    (1)? 0xffffffffffffffffUll: (void *)0;
    (1)? 0x10000000000000000LLU: (void *)0;
}
