#li\
ne 1\
00

#define foo(x) x
#define bar q(
#define q()

#define line 1000
#define fline() 1000

#line 500\
ab
visible
  #  line fline()
foo(bar)
visible

 #\
line 10 "foo.c xtra tokens
 # line 10 "foo.c" xtra tokens !!
