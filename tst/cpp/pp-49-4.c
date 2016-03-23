#if "x" && "y" || "z"
#endif

#if 1 && 2 || "x"
#endif
#if 1 && "x" || 2
#endif


#if 'x' && 'y' || 'z'
#endif

#if 1 && 2 || 'x'
#endif
#if 1 && 'x' || 2
#endif


#if defined x && defined y || defined z
#endif

#if x && y && z || a
#endif
#if x || y && z && a
#endif


#if 1++ && 2-- || 3[0]
#endif

#if 1 && 2 || 3++
#endif


#if *1 && ++2 || &3
#endif

#if 1 && 2 || --3
#endif


#if 1 += 2 && 2 -= 3 || 3 *= 4
#endif

#if 1 && 2 || 3 *= 4
#endif


#if 1 && 2 || 3? 1: 0
#endif
#if 1? 1 && 2 || 3: 0
#endif
#if 0? 0: 1 && 2 || 3
#endif
#if 1 && 2 || 3? 1 && 2 || 3: 1 && 2 || 3
#endif

#if 1 = 2 && 2 = 3 || 3 = 4
#endif
#if 1 && 2 || 3 = 4
#endif

#if 1, 2 && 3 || 4
#endif
#if 1 && 2 || 3, 4
#endif
#if 1 && 2 || 3, 4 && 5 || 6
#endif
