#define foo
#ifdef foo    /* ... */

!this part is to be ignored
this line is not # directive
never ever #else
!this is also to be ignored
    #    else    /* the real one */
!this is not ignored
  /* ... */    this is not recognized as #else
- #endif
/* real */ #endif

# endif
