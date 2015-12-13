int g(int a, int b)
{
}

void f(void)
{
    struct {
        int a, b, c;
        struct {
            unsigned s;
        } *w;
    } *tsym;

    tsym->b = g(tsym->b, tsym->w->s);
}

int main(void)
{
}
