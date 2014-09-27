#if 1? 0: ('b'? 1/0: 0)
#endif
#if 1 || 1 || 0 || 1/0
#endif
#if 1 && 0 && 0 && 1 && 1/0
#endif
#if 0 || 0 || 0 || 1/0    /* warning */
#endif
#if 1 && 1 && 1/0    /* warning */
#endif
#if 1 && 1 && (0 || !0) && 1 && (!1 || !2 || -1) && 1/0    /* warning */
#endif
