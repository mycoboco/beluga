#define xstr(a) str(a)
#define str(a) #a
#define paste(a, b) a ## b
#define paste3(a, b, c) a ## b ## c

paste(+, =)
paste(%, =)
paste(?, =)
paste3(+, =, =)
paste3(*, =, =)
paste3(?, =, =)

str(+=)
str(?=)
xstr(paste(-, =))
xstr(paste(?, =))
xstr(paste3(&, =, =))
xstr(paste3(?, =, =))
