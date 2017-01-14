static int foo;

void f(void)
{
    int foo;
    {
        extern int foo;
    }
}

int foo;

int fred;
static int fred;

extern int bar;
static int bar;

static int goo;
extern int goo;
int goo;

void g(void)
{
    extern int hoo;
}

static int hoo;
