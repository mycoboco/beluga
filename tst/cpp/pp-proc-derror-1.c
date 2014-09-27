#define very VERY
#error
#error	
#error         
#error    /* ... */
#error/
#error/			  
#error/    /* ... */
#error/0"abc"
#error short test
#error short test    /* ... */
#error "string\n", ?#    3.14u .15e-1p        /* ... */ / * / /
#error "string\n", ?#    3.14u .15e-1p        /* ... */ / * / / very   very very very very very very very very long "very very very    long long long"
#error
#if 0
#error never printed out
#else
#if 1
#error printed
#else
#error not printed
#endif
#endif
