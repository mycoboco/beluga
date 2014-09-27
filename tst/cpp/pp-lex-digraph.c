%:define str(x) %:x
#define xstr(x) str(x)
%:define err1(x) %:a
%: define err2(x) %: a
 %:define err3(x) x %:
%:define paste(a, b) a%:%:b
 %: define err4(a, b) %:%:
%: define err5(a, b) a%:%:
%:define err6(a, b) %:%:b

#define test1 #paste(#,#)#paste(#,#)
#define test2 <paste(%:,%:)<paste(%:,%:)
#define test3 %##:%%:%::

test1
test2
test3

str(paste(##, %:))
xstr(paste(%:, >))

%: if array<:0:><:1:>
%:elif <% 1 + 1 %>
#endif
