struct test {
    void *type;
    struct {
        int y;
        int x;
    } pos;
};

void f(char *p, char *q, int n)
{
    *(struct test *)(q+n) = *(struct test *)(p+n);
}

int main(void)
{
}
