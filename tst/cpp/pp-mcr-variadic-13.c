
/* from https://stackoverflow.com/questions/4054085/appending-to-va-args/9204947#9204947 */

#define MACRO(api, ...) \
    bool ret = api(__VA_ARGS__ VA_COMMA(__VA_ARGS__) 456)

/*
 *  VA_COMMA() expands to nothing if given no arguments and a comma if
 *  given 1 to 4 arguments.  Bad things happen if given more than 4
 *  arguments.  Don't do it.
 */
#define VA_COMMA(...) GET_6TH_ARG(,##__VA_ARGS__,COMMA,COMMA,COMMA,COMMA,)
#define GET_6TH_ARG(a1,a2,a3,a4,a5,a6,...) a6
#define COMMA ,

/* EXAMPLES */
MACRO(foo)                       /* bool ret = foo( 456)              */
MACRO(foo,1)                     /* bool ret = foo(1 , 456)           */
MACRO(foo,1,2,3,4)               /* bool ret = foo(1,2,3,4 , 456)     */
/* uh oh, too many arguments: */
MACRO(foo,1,2,3,4,5)             /* bool ret = foo(1,2,3,4,5 5 456)   */
MACRO(foo,1,2,3,4,5,6)           /* bool ret = foo(1,2,3,4,5,6 5 456) */
