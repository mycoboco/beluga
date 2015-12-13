/* -Wv --std=c99 */

int a1[(3.14, 1)];
int a2[(int)(1, 3.14)];
int a3[(3.14, 1, 1)];
int a4[(a1, 1)];
int a5[(1, a1, 1)];
int a6[((char)1, (unsigned char)1)];
int a7[("abc", 1)];

int a8[1 || (2, 3)];                         /* okay */
int a9[1 || (3.14, 1)];
int a10[1 || (a1, 1)];
int a11[1 || ((char)1, (unsigned char)1)];    /* okay */
int a12[1 || ("abc", 1)];

int x1 = (3.14, 1);
int x2 = (1, 3.14);
int x3 = 0 && (3.14, 1);    /* okay */
int x4 = 0 && (1, 3.14);    /* okay */
int x5 = ("abc", 1);
int x6 = (1, "abc");
int x7 = 1 || ("abc", 1);
int x8 = 1 || (1, "abc");
int x9 = ((void *)0, 1);
int x10 = (1, (void *)0);
int x11 = 1 || ((void *)0, 1);
int x12 = 1 || (1, (void *)0);
