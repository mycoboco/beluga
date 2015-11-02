/*
 *  text (cbl)
 */

#ifndef TEXT_H
#define TEXT_H


/* text */
typedef struct text_t {
    int len;            /* length */
    const char *str;    /* string */
} text_t;

/* top of stack-like text space */
typedef struct text_save_t text_save_t;


/* predefined texts */
extern const text_t text_ucase;
extern const text_t text_lcase;
extern const text_t text_digits;
extern const text_t text_null;


text_t text_put(const char *);
text_t text_gen(const char *, int);
text_t text_box(const char *, int);
char *text_get(char *, int, text_t);
int text_pos(text_t, int);
text_t text_sub(text_t, int, int);
text_t text_cat(text_t, text_t);
text_t text_dup(text_t, int);
text_t text_reverse(text_t);
text_t text_map(text_t, const text_t *, const text_t *);
int text_cmp(text_t, text_t);
int text_chr(text_t, int, int, int);
int text_rchr(text_t, int, int, int);
int text_upto(text_t, int, int, text_t);
int text_rupto(text_t, int, int, text_t);
int text_any(text_t, int, text_t);
int text_many(text_t, int, int, text_t);
int text_rmany(text_t, int, int, text_t);
int text_find(text_t, int, int, text_t);
int text_rfind(text_t, int, int, text_t);
int text_match(text_t, int, int, text_t);
int text_rmatch(text_t, int, int, text_t);
text_save_t *text_save(void);
void text_restore(text_save_t **);


/* accesses character with position */
#define TEXT_ACCESS(t, i) ((t).str[((i) <= 0)? (i)+(t).len: (i)-1])


#endif    /* TEXT_H */

/* end of text.h */
