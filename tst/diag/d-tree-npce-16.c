int i;

int *p1 = &i + 3 + (int)(3.14+0);
int *p2 = &i + (int)(3.14+3.14) + 3;
int *p3 = (int)(3.14+3.14) + 3 + &i;
int *p4 = (int)(3.14+3.14) + &i + 3;

int *p5 = &i - (unsigned)(double)3 + (int)(3.14+0.0);
int *p6 = &i - (int)(3.14+0) - (unsigned)(double)3;
int *p7 = (int)(3.14+0) + &i - (unsigned)(double)3;

int i1 = &i - &i;
int i2 = &i - (int *)0;
int i3 = (int *)4 - (int *)0;
int i4 = (int)(int *)0;
