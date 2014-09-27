#define x() q # x
#define y() q#y
#define w() a # b
#define z() a#b

x
y
w
z

#define i q # i
#define j q#i
#define k a # b
#define l a#b

i
j
k
l
