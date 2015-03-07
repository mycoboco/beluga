/*
 *  macro guard optimization
 */

#ifndef MG_H
#define MG_H


/* macro guard state */
enum {
    MG_SINIT,      /* initial or cannot optimize */
    MG_SIFNDEF,    /* met #ifndef */
    MG_SMACRO,     /* remembered a macro */
    MG_SENDIF,     /* met #endif; can optimize if EOI follows */
    MG_SINCLUDE    /* after #include */
};


extern int mg_state;           /* macro guard state */
extern const char *mg_name;    /* macro for #include guard */


void mg_once(void);
int mg_isguarded(const char *);


#endif    /* MG_H */

/* end of mg.h */
