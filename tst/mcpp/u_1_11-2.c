
/* u_1_11.c:    Undefined behaviors on undefined #include syntax or header-
        name.   */

main( void)
{
/*  \ is a legal path-delimiter in MS-DOS or some other OS's.   */
#include    "..\test-t\line.h"

    return  0;
}

