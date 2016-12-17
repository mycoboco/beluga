/*
 *  stop set
 */

#ifndef SSET_H
#define SSET_H


/* predefined stop sets to handle syntax errors */
extern const char sset_field[];
extern const char sset_strdef[];
extern const char sset_enumdef[];
extern const char sset_decl[];
extern const char sset_declf[];
extern const char sset_declb[];
extern const char sset_expr[];
extern const char sset_exprasgn[];
extern const char sset_initf[];
extern const char sset_initb[];


void sset_skip(int, const char []);
void sset_expect(int);
void sset_test(int, const char []);


#endif    /* SSET_H */

/* end of sset.h */
