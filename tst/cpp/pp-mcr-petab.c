#define s1(x) #x
#define s2(x) #x x
#define s3(x) !!
#define s4(x) x ## x
#define s5(x, y) #y x
#define s6(x) x#x x
#define paste(a, b) a ## b

paste(!, !)
s1(paste(!, !))
s2(paste(!, !))
s3(paste(!, !))
s4(paste(!, !))
s5(paste(!, !), !)
s5(!, paste(!, !))
s6(paste(!, !))
