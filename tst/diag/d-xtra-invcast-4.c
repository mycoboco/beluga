int z;
struct tag { int x; } x;
struct tag g();
void (*fp)();

void f(void)
{
    /* struct to integer */
    z = (int)x + 1;
    z = (int)x % (int)x;
    z = ((int *)x)[(int)x];
    z = (int)x > (int)x;
    z = +(int)x;
    z = (int)g() + 1;
    z = (int)g() % (int)g();
    z = ((int *)g())[(int)g()];
    z = (int)g() > (int)g();
    z = +(int)g();
    z = (int)(struct tag)1 + 1;
    z = (int)(struct tag)1 % (int)(struct tag)1;
    z = ((int *)(struct tag)1)[(int)(struct tag)1];
    z = (int)(struct tag)1 > (int)(struct tag)1;
    z = +(int)(struct tag)1;

    /* struct to fp */
    z = (float)x + 0.1;
    z = (float)x + (float)x;
    z = (float)x > (float)x;
    z = -(float)x;
    z = (float)g() + 0.1;
    z = (float)g() + (float)g();
    z = (float)g() > (float)g();
    z = -(float)g();
    z = (float)(struct tag)1 + 0.1;
    z = (float)(struct tag)1 + (float)(struct tag)1;
    z = (float)(struct tag)1 > (float)(struct tag)1;
    z = -(float)(struct tag)1;

    /* struct to ptr */
    z = ((int *)x)[1];
    ((int *)x)[1] = 1;
    z = ((int *)x) - ((int *)x);
    (*((int *)x))++;
    z = ((int *)g())[1];
    ((int *)g())[1] = 1;
    z = ((int *)g()) - ((int *)g());
    (*((int *)g()))++;
    z = ((int *)(struct tag)1)[1];
    ((int *)(struct tag)1)[1] = 1;
    z = ((int *)(struct tag)1) - ((int *)(struct tag)1);
    (*((int *)(struct tag)1))++;

    /* struct to function */
    ((void ())x)(0, 1);
    ((void ())g())(0, 1);
    ((void ())(struct tag)1)(0, 1);
    fp = ((void ())x);
    fp = ((void ())g());
    fp = ((void ())(struct tag)1);

    /* struct to array */
    z = ((int [10])x)[0];
    ((int [10])x)[0] = 1;
    z = ((int [10])(struct tag)1)[1];
    ((int [10])(struct tag)1)[1] = 1;
    z = ((int [10])g())[2];
    ((int [10])g())[2] = 1;
}
