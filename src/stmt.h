/*
 *  statement parsing
 */

#ifndef STMT_H
#define STMT_H

#ifndef NDEBUG
#include <stdio.h>    /* FILE */
#endif    /* !NDEBUG */

#include "dag.h"
#include "gen.h"
#include "lex.h"
#include "sym.h"
#include "tree.h"


/* codelist entry kinds */
enum {
    STMT_BLOCKBEG,    /* beginning of block */
    STMT_BLOCKEND,    /* end of block */
    STMT_LOCAL,       /* local symbol */
    STMT_ADDRESS,     /* address symbol */
    STMT_DEFPOINT,    /* execution point */
    STMT_LABEL,       /* label */
    STMT_START,       /* beginning of code list */
    /* followings generate executable code */
    STMT_GEN,         /* expression or statement */
    STMT_JUMP,        /* branch */
    STMT_SWITCH       /* branch table or placeholder for switch */
};

typedef struct stmt_swtch_t stmt_swtch_t;    /* siwtch handle; opaque type */

/* statement list entry */
typedef struct stmt_t {
    char kind;                     /* kind of codelist entry */
    struct stmt_t *prev, *next;    /* links list */
    union {
        struct {
            int scope;           /* scope of block */
            void **local;        /* (sym_t) locals declared in block */
            sym_tab_t *ident,    /* identifier table for block */
                      *type;     /* type table for block */
            gen_env_t x;         /* info that should be kept for block */
        } block;                 /* for BLOCKBEG */
        struct stmt_t *begin;    /* for BLOCKEND */
        sym_t *var;    /* for LOCAL */
        struct {
            sym_t *sym;     /* generated address symbol */
            sym_t *base;    /* base symbol */
            long offset;    /* offset from base symbol */
        } addr;             /* for ADDRESS */
        struct {
            lex_pos_t pos;    /* locus for exec point */
            int point;        /* unique value for exec point */
        } point;              /* for DEFPOINT */
        dag_node_t *forest;    /* for LABEL, GEN, JUMP */
        struct {
            sym_t *sym;       /* symbol for switch value */
            sym_t *table;     /* branch table */
            sym_t *deflab;    /* symbol for default label */
            int size;         /* size of branch table */
            long *value;      /* case value-label pair */
            sym_t **label;
        } swtch;              /* for SWITCH; different from switch handle */
    } u;
} stmt_t;


extern stmt_t stmt_head;       /* head for statement list */
extern stmt_t *stmt_list;      /* statment list */
extern double stmt_density;    /* density for branch table */
extern sym_tab_t *stmt_lab;    /* symbol table for source-code label */


void stmt_retcode(tree_t *, const lex_pos_t *);
stmt_t *stmt_new(int);
void stmt_local(sym_t *);
void stmt_defpoint(const lex_pos_t *);
void stmt_deflabel(int);
void stmt_eqlabel(sym_t *, sym_t *);
dag_node_t *stmt_jump(int);
void stmt_chkreach(void);
void stmt_stmt(int, stmt_swtch_t *, int, const lex_pos_t *, int *, int);
#ifndef NDEBUG
void stmt_print(FILE *);
#endif    /* !NDEBUG */


#endif    /* STMT_H */

/* end of stmt.h */
