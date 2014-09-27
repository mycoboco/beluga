#define if     if foo
#define ifdef  ifdef foo
#define ifndef ifndef foo
#define elif   elif foo
#define else   else foo
#define endif  endif foo
#define define define foo
#define undef  undef foo
#define line   line foo
#define error  error foo
#define pragma pragma foo

#if 1
if
endif
#endif

#ifdef ifdef
ifdef
#endif

#ifndef not
ifndef
#endif

#if 0
#elif 1
elif
#endif

#ifndef else
#else
else
#endif

define
#undef define
undef define

#line 1
line

#pragma
pragma

error
#error end
