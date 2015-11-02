/*
 *  list (cdsl)
 */

#ifndef LIST_H
#define LIST_H

#include <stddef.h>    /* size_t */


/* list node */
typedef struct list_t {
    void *data;             /* data */
    struct list_t *next;    /* next node */
} list_t;


list_t *list_list(void *, ...);
list_t *list_append(list_t *, list_t *);
list_t *list_push(list_t *, void *);
list_t *list_copy(const list_t *);
list_t *list_pop(list_t *, void **);
void **list_toarray(const list_t *, void *);
size_t list_length(const list_t *);
void list_free(list_t **);
void list_map(list_t *, void (void **, void *), void *);
list_t *list_reverse(list_t *);


/* iterates for each node in list */
#define LIST_FOREACH(pos, list) for ((pos) = (list); (pos); (pos)=(pos)->next)


#endif    /* LIST_H */

/* end of list.h */
