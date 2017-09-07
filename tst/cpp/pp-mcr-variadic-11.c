
/* from https://en.wikipedia.org/wiki/Variadic_macro */

# define MYLOG(FormatLiteral, ...)  fprintf (stderr, "%s(%d): " FormatLiteral "\n", __FILE__, __LINE__, __VA_ARGS__)

MYLOG("Too many balloons %u", 42);
MYLOG("Attention!");
