#define before_scon(x)  x"foo"
#define after_scon(x)   "foo"x

#define before_pcnt(x)  x%
#define after_pcnt(x)   %x

#define before_colon(x) x:
#define after_colon(x)  :x

#define before_carot(x) x^
#define after_carot(x)  ^x

#define before_le(x)    x<=
#define after_le(x)     <=x

#define before_ne(x)    x!=
#define after_ne(x)     !=x

#define before_star(x)  x*
#define after_star(x)   *x

#define before_dot(x)   x.
#define after_dot(x)    .x

#define before_land(x)  x&&
#define after_land(x)   &&x

#define before_ell(x)   x...
#define after_ell(x)    ...x

#define before_cbxor(x) x^=
#define after_cbxor(x)  ^=x

before_scon(L)
before_scon(id)
before_scon(float)
before_scon("bar")
before_scon('x')
before_scon(--)
before_scon(.)
before_scon(^)

after_scon("bar")
after_scon(foo)
after_scon('x')
after_scon(%)
after_scon(&)
after_scon(++)

/* digraphs */
before_pcnt(<)
after_pcnt(>)
after_pcnt(:)
before_colon(<)
before_colon(%)
after_colon(>)

before_carot(^)
before_carot(=)
after_carot(^)
after_carot(=)

before_le(<)
before_le(--)
after_le(=)
after_le(?)

before_ne(!)
after_ne(=)

before_star(/)
before_star(?)
after_star(=)
after_star(])
after_star(*)

before_dot(..)
before_dot(...)
before_dot(3)
after_dot(..)
after_dot(...)
after_dot(14)

before_land(&)
before_land(?)
after_land(&)

before_ell(3)
before_ell(.)
after_ell(.)

before_cbxor(^)
before_cbxor(?)
after_cbxor(=)
