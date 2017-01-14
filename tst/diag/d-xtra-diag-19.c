typedef int *pint;

struct tag {
    pint x: 1;
    double y: 1;
    int f(): 1;
    float a[]: 1;
    char c: 1;
    unsigned char :1;
    int *(): 1;
};
