#define str(x) #x
#define paste1(x, y) x ## y
#define paste2(x, y, z) x ## y ## z
#define strp(x, y) x ## #y

#define invstr(f) f(\"\\"\)
#define invpaste(f) f(abcdefghijk, .)
#define deporder(f) f(123456789, e, +)
#define PASTE paste2

invstr(str)
invpaste(paste1)
deporder(paste2)
deporder(paste2,
    extra /* ... */
)
deporder(PASTE, EXTRA)
invpaste(strp)
invpaste(strp, extra)

#define eval(e) e
#define mcr(x, q) x(123456789 / q)

#if (1/0) + mcr(\
eval\
, 0\
) + (1/0)
#endif

#define manyarg(x) x(abcdefg, hijklmn)
manyarg(str)

#define head str(
#define unterm(x) x(head)
unterm(eval)

#define fewarg1(x) x(abcdefg)
fewarg1(paste1)

#define fewarg2(x) x(abcdefg, )
fewarg2(paste2)

#define id(x) x
#define call(f) f(defined)
#if call(id) id
anyway
#endif
