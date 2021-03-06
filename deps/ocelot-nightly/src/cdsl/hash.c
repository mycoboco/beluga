/*
 *  hash (cdsl)
 */

#include <stddef.h>    /* size_t, NULL */
#include <stdio.h>     /* sprintf */
#include <string.h>    /* memcmp, memcpy, strlen */
#include <limits.h>    /* CHAR_BIT, UCHAR_MAX */
#include <stdarg.h>    /* va_list, va_start, va_arg, va_end */

#include "cbl/assert.h"    /* assert with exception support */
#include "cbl/memory.h"    /* MEM_ALLOC, MEM_FREE */
#include "hash.h"

#if UCHAR_MAX > 255
#error "scatter[] assumes UCHAR_MAX < 256!"
#endif


/*
 *  list for hash strings
 *
 *  In this implementation, storage for str is allocated together when storage for struct hash_t
 *  allocated. To be precise, the size of storage allocated for struct hash_t is not that of
 *  struct hash_t, but includes len. Then the pointer str points to the extra space. This is
 *  dipicted as follows:
 *
 *                           |--------- len ---------|
 *      +------+------+------+-----------------------+
 *      | link | len  | str  |                       |
 *      +------+------+------+-----------------------+
 *                       |    | str points to the beginning of the extra space
 *                       +----+
 *
 *  This structure simplifies (de)allocation of storages for hash nodes, and enables the memory
 *  layout used by some pointer tricks; see hash_length().
 *
 *  In the original implementation, there was no facilities to deallocate storages for hash nodes
 *  because in general the lifetime of a hash string persisted during the exceution of the program;
 *  it is not an error according to the C standard.
 */
struct hash_t {
    struct hash_t *link;    /* next hash string */
    size_t len;             /* length of hash string */
    char *str;              /* hash string */
};


/* buckets for hash string lists;
   size of bucket should be power of 2 (see hash_new()) */
static struct hash_t *bucket[2048];


/*
 *  table to map characters to random numbers
 *
 *  scatter is used to map to a random number a character of unsigned char type given as an array
 *  index. The numbers are generated by the standard's rand(). This scatter table need to be
 *  revised when the number of characters in an implementation is greater than 256. Because the
 *  hash numbers are calculated using scatter, the size of bucket can have a less sophisticated
 *  form than that in the table/set library.
 */
static unsigned long scatter[] = {
    2078917053,  143302914, 1027100827, 1953210302,  755253631, 2002600785, 1405390230,   45248011,
    1099951567,  433832350, 2018585307,  438263339,  813528929, 1703199216,  618906479,  573714703,
     766270699,  275680090, 1510320440, 1583583926, 1723401032, 1965443329, 1098183682, 1636505764,
     980071615, 1011597961,  643279273, 1315461275,  157584038, 1069844923,  471560540,   89017443,
    1213147837, 1498661368, 2042227746, 1968401469, 1353778505, 1300134328, 2013649480,  306246424,
    1733966678, 1884751139,  744509763,  400011959, 1440466707, 1363416242,  973726663,   59253759,
    1639096332,  336563455, 1642837685, 1215013716,  154523136,  593537720,  704035832, 1134594751,
    1605135681, 1347315106,  302572379, 1762719719,  269676381,  774132919, 1851737163, 1482824219,
     125310639, 1746481261, 1303742040, 1479089144,  899131941, 1169907872, 1785335569,  485614972,
     907175364,  382361684,  885626931,  200158423, 1745777927, 1859353594,  259412182, 1237390611,
      48433401, 1902249868,  304920680,  202956538,  348303940, 1008956512, 1337551289, 1953439621,
     208787970, 1640123668, 1568675693,  478464352,  266772940, 1272929208, 1961288571,  392083579,
     871926821, 1117546963, 1871172724, 1771058762,  139971187, 1509024645,  109190086, 1047146551,
    1891386329,  994817018, 1247304975, 1489680608,  706686964, 1506717157,  579587572,  755120366,
    1261483377,  884508252,  958076904, 1609787317, 1893464764,  148144545, 1415743291, 2102252735,
    1788268214,  836935336,  433233439, 2055041154, 2109864544,  247038362,  299641085,  834307717,
    1364585325,   23330161,  457882831, 1504556512, 1532354806,  567072918,  404219416, 1276257488,
    1561889936, 1651524391,  618454448,  121093252, 1010757900, 1198042020,  876213618,  124757630,
    2082550272, 1834290522, 1734544947, 1828531389, 1982435068, 1002804590, 1783300476, 1623219634,
    1839739926,   69050267, 1530777140, 1802120822,  316088629, 1830418225,  488944891, 1680673954,
    1853748387,  946827723, 1037746818, 1238619545, 1513900641, 1441966234,  367393385,  928306929,
     946006977,  985847834, 1049400181, 1956764878,   36406206, 1925613800, 2081522508, 2118956479,
    1612420674, 1668583807, 1800004220, 1447372094,  523904750, 1435821048,  923108080,  216161028,
    1504871315,  306401572, 2018281851, 1820959944, 2136819798,  359743094, 1354150250, 1843084537,
    1306570817,  244413420,  934220434,  672987810, 1686379655, 1301613820, 1601294739,  484902984,
     139978006,  503211273,  294184214,  176384212,  281341425,  228223074,  147857043, 1893762099,
    1896806882, 1947861263, 1193650546,  273227984, 1236198663, 2116758626,  489389012,  593586330,
     275676551,  360187215,  267062626,  265012701,  719930310, 1621212876, 2108097238, 2026501127,
    1865626297,  894834024,  552005290, 1404522304,   48964196,    5816381, 1889425288,  188942202,
     509027654,   36125855,  365326415,  790369079,  264348929,  513183458,  536647531,   13672163,
     313561074, 1730298077,  286900147, 1549759737, 1699573055,  776289160, 2143346068, 1975249606,
    1136476375,  262925046,   92778659, 1856406685, 1884137923,   53392249, 1735424165, 1602280572
};


/*
 *  returns a hash string for a string
 *
 *  The masking macro for hash_string() is not provided because the macro would contain str twice.
 */
const char *(hash_string)(const char *str)
{
    assert(str);
    return hash_new(str, strlen(str));
}


/*
 *  returns a hash string for a signed integer
 */
const char *(hash_int)(long n)
{
    int len;
    char str[1 + (sizeof(long)*CHAR_BIT+2)/3 + 1];
             /* sign + possible number of octal digits in long + null character */

    len = sprintf(str, "%ld", n);    /* len does not count null character */

    return hash_new(str, len);
}


/*
 *  returns a hash string for a byte sequence
 *
 *  Adjusting h into the valid range of bucket originally should be performed by the remainder
 *  operator (%). Because on most implementations it runs slow, the operation is replaced by the
 *  bitwise AND operator (&) with an assumption that the size of bucket is a power of 2. In
 *  general, it is said that choosing a power of 2 as the bucket size for hashing leads to
 *  performance degration due to frequent conflicts. In this implementation, however, a power of 2
 *  is chosen for speeding hash_new() up with scatter that helps to restrain conflicts; profiling
 *  on various applications reported that hash_new() is heavily used so worth being optimized.
 */
const char *(hash_new)(const char *byte, size_t len)
{
    size_t i;
    unsigned long h;
    struct hash_t *p;

    assert(byte);

    /* hashing */
    for (h = 0, i = 0; i < len; i++)
        h = (h << 1) + scatter[(unsigned char)byte[i]];
    h &= (sizeof(bucket)/sizeof(*bucket)-1);

    /* check if hash string already exists */
    for (p = bucket[h]; p; p = p->link)
        if (len == p->len && memcmp(p->str, byte, len) == 0)
                return p->str;

    /* not exist, so creates new one */
    p = MEM_ALLOC(sizeof(*p) + len + 1);    /* +1 for null character */
    p->len = len;
    p->str = (char *)(p + 1);    /* note that p is of struct hash_t type here and no violation of
                                    alignment restriction is possible because of type of p->str */
    memcpy(p->str, byte, len);    /* zero len causes no problem */
    p->str[len] = '\0';
    p->link = bucket[h];    /* pushes new node to hash list */
    bucket[h] = p;

    return p->str;
}


/*
 *  returns the length of a hash string
 *
 *  In the original implementation, hash_length() had to look through the entire hash table because
 *  it was impossible to hash a byte sequence without knowing its length, while the revised
 *  implementation (labelled as "fast version") takes advantage of a pointer trick which is more
 *  dangerous.
 */
size_t (hash_length)(const char *byte)
{
#if 1    /* fast but dangerous version */
    struct hash_t *p;

    assert(byte);

    p = (struct hash_t *)byte - 1;

    return p->len;
#else    /* original slow but safe version */
    size_t i;
    struct hash_t *p;

    assert(byte);

    for (i = 0; i < sizeof(bucket)/sizeof(*bucket); i++)
        for (p = bucket[i]; p; p = p->link)
            if (p->str == byte)    /* same address means same hash string */
                return p->len;

    assert(!"invalid hash string -- should never reach here");

    return 0;
#endif    /* two versions */
}


/*
 *  deallocates storage for a hash string
 *
 *  It is impossible to provide a faster version of hash_free() using a pointer trick as done for
 *  hash_length() because the information of the previous node is necessary to maintain the linkage
 *  of a list.
 */
void (hash_free)(const char *byte)
{
    size_t i;
    struct hash_t *p,
                  **pp;    /* double pointer to which next of deleted node to be stored */

    assert(byte);

    for (i = 0; i < sizeof(bucket)/sizeof(*bucket); i++)
        for (p=bucket[i], pp=&bucket[i]; p; pp=&p->link, p=p->link)
            if (p->str == byte) {
                *pp = p->link;
                MEM_FREE(p);
                return;
            }

    assert(!"invalid hash string -- should never reach here");
}


/*
 *  resets the hash table by deallocating all hash strings in it
 */
void (hash_reset)(void)
{
    size_t i;
    struct hash_t *p;

    for (i = 0; i < sizeof(bucket)/sizeof(*bucket); i++) {
        while ((p = bucket[i]) != NULL) {
            bucket[i] = p->link;
            MEM_FREE(p);
        }
    }
}


/*
 *  puts a sequence of strings to the hash table
 */
void hash_vload(const char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    for (; str; str = va_arg(ap, const char *))
        hash_string(str);
    va_end(ap);
}


/*
 *  puts given strings to the hash table
 */
void hash_aload(const char *strs[])
{
    const char *str;

    for (str = strs[0]; str; strs++, str=*strs)
        hash_string(str);
}

/* end of hash.c */
