/* -Wv */
/* from https://github.com/pfultz2/Cloak */

#ifndef CLOAK_GUARD_H
#define CLOAK_GUARD_H

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BITAND(x) PRIMITIVE_CAT(BITAND_, x)
#define BITAND_0(y) 0
#define BITAND_1(y) y

#define INC(x) PRIMITIVE_CAT(INC_, x)
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8
#define INC_8 9
#define INC_9 9

#define DEC(x) PRIMITIVE_CAT(DEC_, x)
#define DEC_0 0
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7
#define DEC_9 8

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,

#define IS_PAREN(x) CHECK(IS_PAREN_PROBE x)
#define IS_PAREN_PROBE(...) PROBE(~)

#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 PROBE(~)

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BOOL(x) COMPL(NOT(x))

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define IF(c) IIF(BOOL(c))

#define EAT(...)
#define EXPAND(...) __VA_ARGS__
#define WHEN(c) IF(c)(EXPAND, EAT)

#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(id) id DEFER(EMPTY)()

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

#define REPEAT(count, macro, ...) \
    WHEN(count) \
    ( \
        OBSTRUCT(REPEAT_INDIRECT) () \
        ( \
            DEC(count), macro, __VA_ARGS__ \
        ) \
        OBSTRUCT(macro) \
        ( \
            DEC(count), __VA_ARGS__ \
        ) \
    )
#define REPEAT_INDIRECT() REPEAT

#define WHILE(pred, op, ...) \
    IF(pred(__VA_ARGS__)) \
    ( \
        OBSTRUCT(WHILE_INDIRECT) () \
        ( \
            pred, op, op(__VA_ARGS__) \
        ), \
        __VA_ARGS__ \
    )
#define WHILE_INDIRECT() WHILE

#define PRIMITIVE_COMPARE(x, y) IS_PAREN \
( \
    COMPARE_ ## x ( COMPARE_ ## y) (())  \
)

#define IS_COMPARABLE(x) IS_PAREN( CAT(COMPARE_, x) (()) )

#define NOT_EQUAL(x, y) \
IIF(BITAND(IS_COMPARABLE(x))(IS_COMPARABLE(y)) ) \
( \
   PRIMITIVE_COMPARE, \
   1 EAT \
)(x, y)

#define EQUAL(x, y) COMPL(NOT_EQUAL(x, y))

#define COMMA() ,

#define COMMA_IF(n) IF(n)(COMMA, EAT)()

#endif


CHECK(PROBE(~)) // Expands to 1
CHECK(xxx) // Expands to 0

#define M(i, _) i
EVAL(REPEAT(8, M, ~)) // 0 1 2 3 4 5 6 7
#undef M

#define A(i, id) \
COMMA_IF(i) \
template<REPEAT(INC(i), B, ~)> class id ## i \
/**/
#define B(i, _) COMMA_IF(i) class


EVAL(REPEAT(3, A, T))
//template< class> class T0 , template< class , class> class T1 , template< class , class , class> class T2

#undef A
#undef B

#define PRED(state, ...) BOOL(state)
#define OP(state, ...) DEC(state), state, __VA_ARGS__
#define MACRO(state) state

EVAL(WHILE(PRED, OP, 8,)) // 0, 1, 2, 3, 4, 5, 6, 7, 8,

#undef PRED
#undef OP
#undef MACRO

#define COMPARE_foo(x) x
#define COMPARE_bar(x) x

PRIMITIVE_COMPARE(foo, bar) // Expands to 1
PRIMITIVE_COMPARE(foo, foo) // Expands to 0

NOT_EQUAL(foo, bar) // Expands to 1
NOT_EQUAL(foo, foo) // Expands to 0
NOT_EQUAL(foo, unfoo) // Expands to 1

EQUAL(foo, bar) // Expands to 0
EQUAL(foo, foo) // Expands to 1
EQUAL(foo, unfoo) // Expands to 0

#undef COMPARE_foo
#undef COMPARE_bar

#define FOREVER() ?  DEFER(FOREVER_INDIRECT) () ()
#define FOREVER_INDIRECT() FOREVER
EVAL(FOREVER())
