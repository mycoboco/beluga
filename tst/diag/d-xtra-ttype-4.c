typedef int myint;
typedef const int cint;
typedef int *ptoint;
typedef myint *ptomyint;
typedef cint *ptocint;
typedef const myint *ptocmyint;
typedef ptomyint aptomyint[10];
typedef ptocint aptocint[10];
typedef aptomyint *paptomyint;
typedef aptocint *paptocint;
typedef struct tag str;

void f(void)
{
    myint Myint;
    cint Cint;
    ptoint Ptoint;
    ptomyint Ptomyint;
    ptocint Ptocint;
    ptocmyint Ptocmyint;
    aptomyint Aptomyint;
    aptocint Aptocint;
    paptomyint Paptomyint;
    paptocint Paptocint;
    str s;

    Myint = s;
    Cint = s;
    Ptoint = s;
    Ptomyint = s;
    Ptocint = s;
    Ptocmyint = s;
    (*Ptoint) = s;
    (*Ptomyint) = s;
    (*Ptocint) = s;
    (*Ptocmyint) = s;
    Aptomyint * s;
    Aptocint * s;
    (*Aptomyint) = s;
    (**Aptomyint) = s;
    (*Aptocint) = s;
    (**Aptocint) = s;
    Paptomyint = s;
    (*Paptomyint) = s;
    (**Paptomyint) = s;
    (***Paptomyint) = s;
    Paptocint = s;
    (*Paptocint) = s;
    (**Paptocint) = s;
    (***Paptocint) = s;
}
