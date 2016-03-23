#if 1 && 2 && 3 || 4
#endif
#if 1 || 2 && 3 && 4
#endif

#if 1 && (2 && 3) || 4
#endif
#if 1 || (2 && 3) && 4
#endif

#if 1 && 2 || 3 && 4 || 5 && 6
#endif
