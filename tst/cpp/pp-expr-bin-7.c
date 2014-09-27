#if 1 || 1 && 1/0
#endif
#if 1 || (1 && 1/0)
#endif
#if (1 || 1) && 1/0    /* warning */
#endif
