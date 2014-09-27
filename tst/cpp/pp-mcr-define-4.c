/* -Wv -DBAR1#BAR=BAR -DBAR2# BAR -DBAR3#B(a)=a */

#define FOO1$
#define FOO2$TEST TEST
#define FOO3(param)$
#define FOO4(param)$TEST
#define FOO5(param)$TEST TEST
#define FOO6(param)TEST$TEST
#define FOO7$(param) param
#define FOO8$TEST(param) param
