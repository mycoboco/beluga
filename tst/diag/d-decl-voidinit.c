void v1;
void v2 = 0;
const void cv = 0 + 2 * 2;
volatile void vv = { 0, { 0, }, };

static void v3;
static void v4 = { 0, };
static const void cv2 = { { 0, }, 0 };
static const volatile void cvv = v3;

void f(int a, void b, void c = 0, const void d = { 0, })
{
    void v5 = 0;
    const void cv3 = { 0, };
    static void v6 = v5;
    static const void cv4 = v1;
    extern void v7 = 1;
    extern volatile void vv2 = v7;
}
