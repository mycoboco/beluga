#foo                  /* unknown */
# foo                 /* unknown */
#veryverylong 123     /* unknown */
# veryverylong 123    /* unknown */
#123                  /* extra */
# 123                 /* extra */
#"abc"                /* extra */
#
#if 0
#foo
#123
#else
#foo                  /* unknown */
#123                  /* extra */
#endif
