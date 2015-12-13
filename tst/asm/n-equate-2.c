static void f(int op)
{
    void *tag;

    struct ty {
        union {
            struct {
                struct pos {
                    int y;
                } pos;
                struct {
                    unsigned int defined: 1;
                } f;
            } *sym;
        } u;
    } *ty;

    extern struct pos *err();

    ty->u.sym->pos = *err();
}
