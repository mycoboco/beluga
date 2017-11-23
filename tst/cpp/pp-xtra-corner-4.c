
/* from http://www.drdobbs.com/open-source/code-finessing/193104882?pgno=3 */

#define A B
#define B C
#define X(val) Y(val)
#define C(a) D((a))

X((A(1)) | (A(2)));
