/*
 *  declaration parsing
 */

#ifndef DECL_H
#define DECL_H

#include "lmap.h"
#include "stmt.h"
#include "sym.h"
#include "ty.h"


extern sym_t *decl_cfunc;     /* function being currently parsed */
extern void **decl_callee;    /* (sym_t) callee of decl_cfunc */
extern sym_t *decl_retv;      /* struct/union return value of decl_cfunc if any */


int decl_chkid(const char *, const lmap_t *, sym_tab_t *, int);
void decl_defglobal(sym_t *, int);
void decl_finalize(void);
void decl_compound(int, stmt_swtch_t *, int);
void decl_program(void);
ty_t *decl_typename(const char *);
void decl_errdecl(const char *);


/* checks missing return value */
#define DECL_NORET(ty) (uty != ty_voidtype &&                                  \
                        (rty->t.type != ty_inttype || !(ty)->u.f.implint ||    \
                         main_opt()->std))


#endif    /* DECL_H */

/* end of decl.h */
