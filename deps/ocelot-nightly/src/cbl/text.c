/*
 *  text (cbl)
 */

#include <stddef.h>    /* NULL */
#include <string.h>    /* memcmp, memcpy, memchr */
#include <limits.h>    /* UCHAR_MAX */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_NEW, MEM_ALLOC, MEM_FREE */

#include "text.h"


/* array index from text position */
#define IDX(i, len) (((i) <= 0)? (i) + (len): (i) - 1)

/*
 *  checks if text can be attached to another text
 *
 *  This library hires a separate allocator to maintain its text space where storages for texts
 *  resides; see alloc(). When a text generated recently occupies the last byte in the storage, an
 *  operation like concatenating another text to it gets easier by simply putting that added text
 *  immediately after the original one in the storage. ISATEND() called "is at end" checks if such
 *  a simple operation is possible for text_dup() and text_cat(). It sees both if the next of the
 *  last byte of a given text starts the available area in the text space and if there is enough
 *  space to contain a text to be attached.
 *
 *  Not all texts reside in the text space. For example, text_box() provides a way to "box" a
 *  string literal to construct its text representation. Even in that case, ISATEND() does not
 *  invoke undefined behavior thanks to the short-circuit logical operator.
 *
 *  The code given here is revised from the original one to avoid generating an invalid pointer
 *  value; the original code adds n to the avail member of current before comparing the result to
 *  the limit member, which renders the code invalid when n is bigger than the remained free space.
 *  The same modification applies to alloc().
 */
#define ISATEND(s, n) ((s).str+(s).len == current->avail && (n) <= current->limit-current->avail)

/* compares sub-text to another text up to length of latter */
#define EQUAL(s, i, t) (memcmp(&(s).str[i], (t).str, (t).len) == 0)

/* swaps two integers */
#define SWAP(i, j) do {              \
                       int t = i;    \
                       i = j;        \
                       j = t;        \
                   } while(0)


/*
 *  save point for text space
 *
 *  The text library provides a limited control over the text space that is storage allocated by
 *  the library. The text space can be seen as a stack and struct text_save_t provides a way to
 *  remember the top of the stack and to restore it later releasing every storage allocated after
 *  the top has been remembered. Comparing struct text_save_t with struct chunk, note that the
 *  member limit is not necessary to remember because it is automatically recovered with current,
 *  but avail is mutable so should be remembered.
 */
struct text_save_t {
    struct chunk *current;    /* chunk to restore later */
    char *avail;              /* start of free area of saved chunk */
};

/*
 *  string space
 *
 *  The text library uses its own storage allocator for:
 *  - it has to support text_save() and text_restore(), and
 *  - it enables some optimization as seen in text_dup() and text_cat().
 *
 *  The text space looks like a single list of chunks in the arena library. It can be even simpler,
 *  however, because there is no need to keep a header information for the space and no need to
 *  worry about the alignment problem; the size of the allocated space is maintained by text_t and
 *  the only type involved with the text space is a character type which has the least alignement
 *  restriction.
 *
 *  The text space starts from the head node whose sole purpose is to let its link member point to
 *  the first chunk allocated if any. Within a chunk, avail points to the beginning of the free
 *  space and limit points to past the end of that chunk. With these two pointers, one can compute
 *  the size of the free space in a chunk.
 */
struct chunk {
    struct chunk *link;    /* next chunk */
    char *avail;           /* start of free area in chunk */
    char *limit;           /* past end of chunk */
};


/* memory chunk list for text space; see alloc() */
static struct chunk head = { NULL, NULL, NULL };

/* last chunk for text space; see alloc() */
static struct chunk *current = &head;

/* character sets; see text_map() */
const text_t text_ucase = { 26, "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
const text_t text_lcase = { 26, "abcdefghijklmnopqrstuvwxyz" };
const text_t text_digits = { 10, "0123456789" };

/* empty text */
const text_t text_null = { 0, "" };


/*
 *  allocates storage in the text space.
 *
 *  Note that the zero size for storage is permitted in order to allow for an empty text. The code
 *  given here differs from the original one in three places:
 *  - it checks if the limit or avail member of current is a null pointer; the original code in the
 *    book did not, which results in undefined behavior;
 *  - an operation adding an integer to a pointer for comparison was revised to that subtracting a
 *    pointer from another pointer (see ISATEND()); and
 *  - a composite assignment expression was decomposed to two separate assignment expressions to
 *    avoid undefined behavior.
 *
 *  The last item deserves more explanation. The original expression:
 *
 *      current = current->link = MEM_ALLOC(sizeof(*current) + 10*1024 + len);
 *
 *  modifies current which is also referenced between two sequence points. Even if it is not
 *  crystal-clear whether or not the reference to current is necessary to determine a new value to
 *  store into current, the literal reading of the standard says undefined behavior about it. Thus,
 *  it seems clever to avoid it.
 */
static char *alloc(int len)
{
    assert(len >= 0);

    if (!current->avail || len > current->limit - current->avail) {
        /* new chunk added to list */
        current->link = MEM_ALLOC(sizeof(*current) + 10*1024 + len);    /* extra 10Kb */
        current = current->link;
        current->avail = (char *)(current + 1);
        current->limit = current->avail + 10*1024 + len;
        current->link = NULL;
    }
    current->avail += len;

    return current->avail - len;
}


/*
 *  normalizes a text position
 *
 *  Note that the relational operator given in the last call to assert() is <=, not < even if the
 *  element referred to by s.str[s.len] does not exist. Because that index is converted from a
 *  valid position for a text, it should not be precluded. For example, the valid position 5 in the
 *  above example is converted to 4 by IDX(). The validity check itself causes no problem because
 *  the non-existing element is never accessed, and a further restrictive check is performed when
 *  there is a chance to access it.
 */
int (text_pos)(text_t s, int i)
{
    assert(s.len >= 0 && s.str);    /* validity check for text */

    i = IDX(i, s.len);    /* converts to index for array */

    assert(i >= 0 && i <= s.len);    /* validity check for position */

    return i + 1;    /* positive position */
}


/*
 *  boxes a null-terminated string to construct a text
 */
text_t (text_box)(const char *str, int len)
{
    text_t text;

    assert(str);
    assert(len >= 0);

    text.str = str;
    text.len = len;

    return text;
}


/*
 *  constructs a sub-text of a text
 */
text_t (text_sub)(text_t s, int i, int j)
{
    text_t text;

    assert(s.len >= 0 && s.str);    /* validity check for text */

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    text.len = j - i;
    text.str = s.str + i;

    return text;
}


/*
 *  constructs a text from a null-terminated string
 */
text_t (text_put)(const char *str)
{
    text_t text;

    assert(str);

    text.len = strlen(str);    /* note that null character not counted */
    text.str = memcpy(alloc(text.len), str, text.len);

    return text;
}


/*
 *  constructs a text from an array of characters
 */
text_t (text_gen)(const char str[], int size)
{
    text_t text;

    assert(str);

    text.len = size;
    text.str = memcpy(alloc(text.len), str, text.len);

    return text;
}


/*
 *  converts a text to a C string
 */
char *(text_get)(char *str, int size, text_t s)
{
    assert(s.len >= 0 && s.str);

    if (str == NULL)
        str = MEM_ALLOC(s.len + 1);    /* +1 for null character */
    else
        assert(size >= s.len + 1);

    memcpy(str, s.str, s.len);
    str[s.len] = '\0';

    return str;
}


/*
 *  constructs a text by duplicating another text
 *
 *  In this implementation, there are two special cases where no real duplication need not occur: n
 *  or the length of a text is 0, and n is 1. Even when duplication is necessary, there is a case
 *  where the original and the resulting texts share the storage. ISATEND() detects such a
 *  case.
 */
text_t (text_dup)(text_t s, int n)
{
    assert(s.len >= 0 && s.str);    /* validity check for text */
    assert(n >= 0);

    if (n == 0 || s.len == 0)
        return text_null;

    if (n == 1)    /* no need to duplicate */
        return s;

    {    /* s.len > 0 && n > 2 */
        text_t text;
        char *p;

        text.len = n*s.len;
        if (ISATEND(s, text.len - s.len)) {    /* possible to duplicate n-1 times and attach */
            text.str = s.str;    /* share storage */
            p = alloc(text.len - s.len);    /* allocates (n-1)*s.len bytes */
            n--;
        } else
            text.str = p = alloc(text.len);
        for (; n-- > 0; p += s.len)
            memcpy(p, s.str, s.len);

        return text;
    }
}


/*
 *  constructs a text by concatenating two texts
 *
 *  As in text_dup(), there are several cases where some optimizations are possible:
 *  - either of two texts is empty, text_cat() can simply return the other,
 *  - if two texts already exist adjacently in the text space, no need to concatenate them, and
 *  - if a text resides at the end of the text space, copying the other there can construct the
 *    result by sharing the storage; ISATEND() detects such a case.
 *
 *  According to the strict interpretation of the C standard, this code might invoke undefined
 *  behavior. If two texts that do not reside in the text space are incidentally adjacent,
 *  text_cat() returns a text representation containing the address of the preceding text, but the
 *  authoritative interpretation of the standard does not guarantee walking through beyond the end
 *  of the preceding text.
 */
text_t (text_cat)(text_t s1, text_t s2)
{
    assert(s1.len >= 0 && s1.str);    /* validity checks for text */
    assert(s2.len >= 0 && s2.str);

    if (s1.len == 0)
        return s2;

    if (s2.len == 0)
        return s1;

    if (s1.str + s1.len == s2.str) {    /* already adjacent texts */
        s1.len += s2.len;
        return s1;    /* possible undefined behavior */
    }

    {    /* s1.len > 0 && s2.len > 0 */
        text_t text;

        text.len = s1.len + s2.len;

        if (ISATEND(s1, s2.len)) {
            text.str = s1.str;
            memcpy(alloc(s2.len), s2.str, s2.len);
        } else {
            char *p;
            text.str = p = alloc(s1.len + s2.len);
            memcpy(p, s1.str, s1.len);
            memcpy(p + s1.len, s2.str, s2.len);
        }

        return text;
    }
}


/*
 *  constructs a text by reversing a text
 *
 *  Two obvious cases where some optimization is possible are that a given text has no or only one
 *  character. Considering characters that signed char cannot represent on implementations with
 *  signed "plain" char, casts to unsigned char * are added.
 */
text_t (text_reverse)(text_t s)
{
    assert(s.len >= 0 && s.str);    /* validity check for text */

    if (s.len == 0)
        return text_null;
    else if (s.len == 1)
        return s;
    else {    /* s.len > 1 */
        text_t text;
        unsigned char *p;
        int i;

        i = text.len = s.len;
        p = (unsigned char *)(text.str = alloc(s.len));
        while (--i >= 0)    /* avoids i-- > 0 to use s.str[i] */
            *p++ = ((unsigned char *)s.str)[i];

        return text;
    }
}


/*
 *  constructs a text by converting a text based on a specified mapping
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, casts to unsigned char * are added.
 */
text_t (text_map)(text_t s, const text_t *from, const text_t *to)
{
    static int inited = 0;    /* indicates if mapping is set */
    static unsigned char map[UCHAR_MAX];

    assert(s.len >= 0 && s.str);    /* validity check for text */

    if (from && to) {    /* new mapping given */
        int k;

        for (k = 0; k < (int)sizeof(map); k++)
            map[k] = k;
        assert(from->len == to->len);    /* validity check for mapping text */
        for (k = 0; k < from->len; k++)
            map[((unsigned char *)from->str)[k]] = ((unsigned char *)to->str)[k];
        inited = 1;
    } else {    /* uses preset mapping */
        assert(from == NULL && to == NULL);
        assert(inited);
    }

    if (s.len == 0)
        return text_null;
    else {
        text_t text;
        int i;
        unsigned char *p;

        text.len = s.len;
        p = (unsigned char *)(text.str = alloc(s.len));
        for (i = 0; i < s.len; i++)
            *p++ = map[((unsigned char *)s.str)[i]];

        return text;
    }
}


/*
 *  compares two texts
 */
int (text_cmp)(text_t s1, text_t s2)
{
    assert(s1.len >= 0 && s1.str);    /* validity checks for texts */
    assert(s2.len >= 0 && s2.str);

    /* if texts share the storage,
           if s1 is longer than s2, return positive
           if s1 and s2 are of the same length, return 0
           if s1 is shorter than s2, return negative
       if texts do not share the storage,
           if the lengths differ,
               compare texts up to the length of shorter text
               if compare equal
                   return negative or positive according to which text is longer
               if compare unequal
                   return the result of memcmp()
           if the lengths equal,
               return the result of memcmp() */

    if (s1.str == s2.str)
        return s1.len - s2.len;
    else if (s1.len < s2.len) {
        int cond = memcmp(s1.str, s2.str, s1.len);
        return (cond == 0)? -1 : cond;
    } else if (s1.len > s2.len) {
        int cond = memcmp(s1.str, s2.str, s2.len);
        return (cond == 0)? +1 : cond;
    } else    /* s1.len == s2.len */
        return memcmp(s1.str, s2.str, s1.len);
}


/*
 *  saves the current top of the text space
 *
 *  TODO:
 *    - text_save() and text_restore() can be improved to detect erroneous calls as shown in the
 *      above example;
 *    - the stack-like storage management by text_save() and text_restore() unnecessarily keeps the
 *      library from being used in ohter libraries. For example, text_restore() invoked by a
 *      clean-up function of a library can destroy the storage for texts that are still in use by a
 *      program. The approach used by the arena library would be more appropriate.
 *
 *  The original code calls alloc() to keep a text from straddling the end of the text space that
 *  is returned by text_save(). This seems unnecessary in this implementation, so commented out.
 */
text_save_t *(text_save)(void)
{
    text_save_t *save;

    MEM_NEW(save);
    save->current = current;
    save->avail = current->avail;
    /* alloc(1); */

    return save;
}


/*
 *  restores a saved state of the text space
 */
void (text_restore)(text_save_t **save)
{
    struct chunk *p, *q;

    assert(save);
    assert(*save);

    current = (*save)->current;
    current->avail = (*save)->avail;
    MEM_FREE(*save);

    for (p = current->link; p; p = q) {
        q = p->link;
        MEM_FREE(p);
    }

    current->link = NULL;
}


/*
 *  finds the first occurrence of a character in a text
 *
 *  Considering characters that sigend char cannot represent and implementations where "plain" char
 *  is signed, a cast to unsigned char * is added.
 */
int (text_chr)(text_t s, int i, int j, int c)
{
    assert(s.len >= 0 && s.str);    /* validity check for text */

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    for (; i < j; i++)    /* note that < is used */
        if (((unsigned char *)s.str)[i] == c)
            return i + 1;

    return 0;
}


/*
 *  finds the last occurrence of a character in a text
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, a cast to unsigned char * is added.
 */
int (text_rchr)(text_t s, int i, int j, int c)
{
    assert(s.len >= 0 && s.str);    /* validity check for text */

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    while (j > i)    /* note that > and prefix -- are used */
        if (((unsigned char *)s.str)[--j] == c)
            return j + 1;

    return 0;
}


/*
 *  finds the first occurrence of any character from a set in a text
 */
int (text_upto)(text_t s, int i, int j, text_t set)
{
    assert(set.len >= 0 && set.str);    /* validity check for texts */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    for (; i < j; i++)    /* note that < is used */
        if (memchr(set.str, s.str[i], set.len))
            return i + 1;

    return 0;
}


/*
 *  finds the last occurrence of any character from a set in a text
 */
int (text_rupto)(text_t s, int i, int j, text_t set)
{
    assert(set.len >= 0 && set.str);    /* validity check for texts */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    while (j > i)    /* note that > and prefix -- are used */
        if (memchr(set.str, s.str[--j], set.len))
            return j + 1;

    return 0;
}


/*
 *  finds the first occurrence of a text in a text
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, casts to unsigned char * are added.
 */
int (text_find)(text_t s, int i, int j, text_t str)
{
    assert(str.len >= 0 && str.str);    /* validity check for texts */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    if (str.len == 0)    /* finding empty text always succeeds */
        return i + 1;
    else if (str.len == 1) {    /* finding-character case */
        for (; i < j; i++)    /* note that < is used */
            if (((unsigned char *)s.str)[i] == *(unsigned char *)str.str)
                return i + 1;
    } else
        for (; i + str.len <= j; i++)    /* note that <= is used and str.len added */
            if (EQUAL(s, i, str))
                return i + 1;

    return 0;
}


/*
 *  finds the last occurrence of a text in a text
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, casts to unsigned char * are added.
 */
int (text_rfind)(text_t s, int i, int j, text_t str)
{
    assert(str.len >= 0 && str.str);    /* validity check for text */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    if (str.len == 0)    /* finding empty text always succeeds */
        return j + 1;
    else if (str.len == 1) {    /* finding-character case */
        while (j > i)    /* note that > and prefix -- are used */
            if (((unsigned char *)s.str)[--j] == *(unsigned char *)str.str)
                return j + 1;
    } else
        for (; j - str.len >= i; j--)    /* note that >= is used and str.len subtracted */
            if (EQUAL(s, j - str.len, str))
                return j - str.len + 1;

    return 0;
}


/*
 *  checks if a character of a specified position matches any character from a set
 */
int (text_any)(text_t s, int i, text_t set)
{
    assert(s.len >= 0 && s.str);    /* validity check for texts */
    assert(set.len >= 0 && set.str);

    i = IDX(i, s.len);

    assert(i >= 0 && i <= s.len);    /* validity check for position; see text_pos() */

    if (i < s.len && memchr(set.str, s.str[i], set.len))    /* note that i < s.len is checked */
        return i + 2;    /* right position, so +2 */

    return 0;
}


/*
 *  finds the end of a span consisted of characters from a set
 */
int (text_many)(text_t s, int i, int j, text_t set)
{
    int t;

    assert(set.len >= 0 && set.str);    /* validity check for text */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    t = i;
    while (i < j && memchr(set.str, s.str[i], set.len))    /* note that i < j is checked */
        i++;

    return (t == i)? 0: i + 1;
}


/*
 *  finds the start of a span consisted of characters from a set
 */
int (text_rmany)(text_t s, int i, int j, text_t set)
{
    int t;

    assert(set.len >= 0 && set.str);    /* validity check for texts */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    t = j;
    while (j > i && memchr(set.str, s.str[j-1], set.len))    /* note that j > i is checked */
        j--;

    return (t == j)? 0: j + 1;
}


/*
 *  checks if a text starts with another text
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, casts to unsigned char * are used.
 */
int (text_match)(text_t s, int i, int j, text_t str)
{
    assert(str.len >= 0 && str.str);    /* validity check for text */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    if (str.len == 0)    /* finding empty text always succeeds */
        return i + 1;
    else if (str.len == 1) {    /* finding-character case */
        if (i < j &&    /* note that < is used */
            ((unsigned char *)s.str)[i] == *(unsigned char *)str.str)
            return i + 2;
    } else if (i + str.len <= j && EQUAL(s, i, str))    /* note that <= is used and str.len added */
        return i + str.len + 1;

    return 0;
}


/*
 *  checks if a text ends with another text
 *
 *  Considering characters that signed char cannot represent and implementations where "plain" char
 *  is signed, casts to unsigned char * are added.
 */
int (text_rmatch)(text_t s, int i, int j, text_t str)
{
    assert(str.len >= 0 && str.str);    /* validity check for texts */
    assert(s.len >= 0 && s.str);

    i = IDX(i, s.len);
    j = IDX(j, s.len);

    if (i > j)
        SWAP(i, j);

    assert(i >= 0 && j <= s.len);    /* validity check for position; see text_pos() */

    if (str.len == 0)    /* finding empty text always succeeds */
        return j + 1;
    else if (str.len == 1) {    /* finding-character case */
        if (j > i &&    /* note that > is used */
            ((unsigned char *)s.str)[j-1] == *(unsigned char *)str.str)
            return j;
    } else if (j - str.len >= i && EQUAL(s, j - str.len, str))    /* note that >= is used and
                                                                     str.len subtracted */
        return j - str.len + 1;

    return 0;
}

/* end of text.c */
