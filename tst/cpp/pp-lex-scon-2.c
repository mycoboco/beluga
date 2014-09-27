/* -Wv --std=c90 -DM1='' -DM2=' ' -DM3=L'' -DM4=L' ' */

''    /* error */
' '
'"'
'\''

""
" "
"'"
"\""

L''    /* error */
L' '
L""
L" "

#if ''    /* error */
#endif
#if ' '
#endif
#if L''    /* error */
#endif
#if L' '
#endif
#if ""
#endif
#if " "
#endif
