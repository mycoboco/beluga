#define scon  "foo"
#define pcnt  %
#define colon :
#define carot ^
#define le    <=
#define ne    !=
#define star  *
#define dot   .
#define land  &&
#define ell   ...
#define cbxor ^=

"bar"scon
'x'scon
--scon
.scon
^scon

scon"bar"
scon'x'
scon%
scon&
scon++

/* digraphs */
<pcnt
pcnt>
pcnt:
<colon
%colon
colon>

^carot
=carot
carot^
carot=

<le
--le
le=
le?

!ne
ne=

/star
star?
star=
star]
star*

..dot
...dot
3dot
dot..
dot...

&land
?land
land&

3ell
.ell
ell.

^cbxor
?cbxor
cbxor=
