void f(void)
{
    /* without check for ty_voidtype in expr_unary(), this creates constant of
       void type in dag_listnode() */
    ((void)1) = 0;
}
