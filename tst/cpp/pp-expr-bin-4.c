/* -Wv --std=c90 --logical-shift */

#if -1 >> 1                /* warning */
#endif
#if -1 >> 1 == ~0U >> 1     /* warning, 1 */
line1
#endif
#if -1 >> 1U == ~0U >> 1    /* warning, 1 */
line2
#endif
