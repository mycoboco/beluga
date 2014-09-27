#define p1(a, b) a##b
#define q1(a, b) a##b a

#define p2(a, b) #a##b 1
#define q2(a, b) #a##b a

#define p3(a, b) a###b 1
#define q3(a, b) a###b b

#define p4(a, b) a##b##a
#define q4(a, b) a##b##a a

#define p5(a, b) a###b###a
#define q5(a, b) a###b###a b

#define p6(a, b) a##b b##a 1
#define q6(a, b) a##b b##a a

#define p7(a, b) a##1##a
#define q7(a, b) a##1##a a

#define p8(a, b) a##b 1##a 1
#define q8(a, b) a##b 1##a a

p1(p1(!,!), !)
q1(p1(!,!), !)

p2(p1(!,!), !)
q2(p1(!,!), !)

p3(!, p1(!,!))
q3(!, p1(!,!))

p4(p1(!,!), !)
q4(p1(!,!), !)

p5(!, p1(!,!))
q5(!, p1(!,!))

p6(p1(!,!), !)
q6(p1(!,!), !)

p7(p1(!,!), !)
q7(p1(!,!), !)

p8(p1(!,!), !)
q8(p1(!,!), !)
