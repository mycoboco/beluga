#define str(x) #x
#define inv(f) f(\"\\"\)

str("")
str('')
str(\)         /* invalid */
str(\\)
str(\"")       /* invalid */
str(\'')
str('\\'\)     /* invalid */
str("\\"\)     /* invalid */
str(\"\x"\)    /* invalid */
str(\'\x'\)    /* invalid */
inv(foo)
inv(str)       /* invalid */
