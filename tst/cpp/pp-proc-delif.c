#define x "foo"

#if 1
  Here
#elif
  No
#elif x
  No
  #if x
  #endif
#endif

#if 0
  No
#elif      /* error */
  No
  #if x
  #endif
#elif x    /* error */
  No
  #if x
  #endif
#endif

#if 1
  Here
  #if 1
  #elif x
  #endif
#elif
  No
  #elif x
#endif

#if 1
  Here
  #if 0
  #elif x    /* error */
  #endif
#elif
  No
  #elif x
#endif

#if 0
#elif      /* error */
#elif x    /* error */
#elif 1
#elif x
#elif
#endif
