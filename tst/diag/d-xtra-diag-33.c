enum bar1 { NO } e1;
enum bar2 { YES } e2;

void f(void)
{
    struct foo { int m; } s;

    (enum bar1)f();
    (struct foo)f();
    (int [])f();
    (void (void))f();

    (enum bar1)s;
    (struct foo)s;
    (int [10])s;
    (int (void))s;

    (enum bar1)e2;
    (struct foo)e2;
    (int [10])e2;
    (int (void))e2;
}
