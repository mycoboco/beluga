/*
 *  doubly-linked list (cdsl)
 */

#ifndef DLIST_H
#define DLIST_H


/* doubly-linked list */
typedef struct dlist_t dlist_t;


dlist_t *dlist_new(void);
dlist_t *dlist_list(void *, ...);
void dlist_free(dlist_t **);
void *dlist_add(dlist_t *, long, void *);
void *dlist_addhead(dlist_t *, void *);
void *dlist_addtail(dlist_t *, void *);
void *dlist_remove(dlist_t *, long);
void *dlist_remhead(dlist_t *);
void *dlist_remtail(dlist_t *);
long dlist_length(const dlist_t *);
void *dlist_get(dlist_t *, long);
void *dlist_put(dlist_t *, long, void *);
void dlist_shift(dlist_t *, long);


#endif    /* DLIST_H */

/* end of dlist.h */
