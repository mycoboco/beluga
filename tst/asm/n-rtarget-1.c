struct test {
    void *type;
    struct {
        int y;
        int x;
    } pos;
};

void f(struct test *p, struct test *q)
{
    p->pos = q->pos;
}

int main()
{
}
