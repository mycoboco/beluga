void f(void)
{
    struct {
        int x:1;
        const int y:1;
    } s = { 0, };

    struct {
        int x:1;
        int y:1;
    } t = { 0 };

    struct {
        int x:1;
    } u = { 0, };

    struct {
        int x:1;
    } v = { 0 };

    struct {
        const int x:1;
        int y;
    } w = { 0, };

    struct {
        int x:1;
        int y;
    } x = { 0 };
}
