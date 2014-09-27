/* -Wv --std=c90 -DFOO="foo -DBAR"bar -DF(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G)= */
#define EXPAND(n) n(1, 2)
FOO
BAR
F(1)
EXPAND(F)
