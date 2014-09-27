#define f(a, b, c, d, e, f) a b c d e f

f(
 # define foo,                 /* warning */
    #                          /* warning */
, test start   /* ... */
 # /* ... */ include # here    /* warning */
test end,
#if 0                          /* warning */
,
  ## #,
not #
)

foo
