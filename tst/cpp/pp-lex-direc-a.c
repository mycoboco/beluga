/* -Wv --std=c90 -DM1testtest */
#defineFOOfoo
#defineBARbarbar

FOOBARFOO
M1

#include "pp-lex-direc-b.c"
#if 1+0
#line100
#elsedummy
#line200
#endif
ordinaryline

#define foo 
#pragmafoo
#pragma bar 
ordinary line
