# if FOO = 2
#endif
  #if BAR <<= 2
#endif
#if FOOBAR += 3
#endif

#if 3 -= (3 / 0)
#endif
#   if   2 *= 0
#endif

#if 0xff /= 0
#endif
#if 0xffff %= 0
#endif
#if -1 <<= -1
#endif
#if -1 >>= -1
#endif
#if -1 |= 0
#endif
#if -1 &= 0
#endif
#if -1 ^= -1
#endif
