#if foo++
#endif
#if foo--
#endif
#if 0++
#endif
#if 0--
#endif
#if (1 / -1)++
#endif
#if (1 * -1)--
#endif
#if foo[1]
#endif
#if 1[foo]
#endif
#if (1 + 1)[1 - 1]
#endif
#if f(arg)
#endif
#if 0(arg)
#endif
#if f(1, 2, 3)
#endif
#if 0(1, 2, 3)
#endif
#if foo.mem
#endif
#if foo.1
#endif
#if 0.bar
#endif
#if (0).(1)
#endif
#if 0.(bar)
#endif
#if foo->mem
#endif
#if foo->1
#endif
#if 0->bar
#endif
#if (0)->(1)
#endif
#if 0->(bar)
#endif
