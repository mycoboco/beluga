/*
 *  ec.h generator
 */

#include <stdio.h>    /* printf */

#define str(s) #s


#define SEA_CANARY
const char *pname[] = {
#define xx(a, b, c, d) str(EP_##a),
#define yy(a, b, c, d) str(EP_##a),
#include "../lib/xerror.h"
};
#undef SEA_CANARY

const char *cname[] = {
#define xx(a, b, c, d) str(EC_##a),
#define yy(a, b, c, d) str(EC_##a),
#include "../lib/xerror.h"
};


int main(void)
{
    int i;

    for (i = 0; i < sizeof(pname)/sizeof(*pname); i++)
        printf("#define %s %d\n", pname[i], i);
    for (i = 0; i < sizeof(cname)/sizeof(*cname); i++)
        printf("#define %s %d\n", cname[i], i);

    return 0;
}

/* end of ecgen.c */
