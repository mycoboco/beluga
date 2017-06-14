/*
 *  table (cdsl)
 */

#ifndef TABLE_H
#define TABLE_H

#include <stddef.h>    /* size_t */


/* table */
typedef struct table_t table_t;


table_t *table_new(int, int (const void *, const void *), unsigned (const void *));
void table_free(table_t **);
size_t table_length(const table_t *);
void *table_put(table_t *, const void *, void *);
void *table_get(const table_t *, const void *);
void *table_remove(table_t *, const void *);
void table_map(table_t *, void (const void *, void **, void *), void *);
void **table_toarray(const table_t *, void *);


#endif    /* TABLE_H */

/* end of table.h */
