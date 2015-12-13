struct {
    int x:1;
    unsigned y:1;
    int a[10];
} f(void)
{
    extern struct tag *g();
    extern void *h();
    extern const void *i();
    extern volatile void *j();

    f().a = 0;
    (f().a = 0) = f;
    f().x = f;
    (f().x = f) = f;
    f().y = f;
    (f().y = f) = f;
    *g() = *g();
    (*g() = *g()) = 0;
    (*h() = *h()) = 0;
    (*i() = *i()) = 0;
    (*j() = *j()) = 0;
}

