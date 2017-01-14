/* -Wv --std=c90 */

#define VOID void
typedef int func();

struct {
    void
};

struct {
    int *ptr;
    int (((ptr)));
    int *ff(), (*ff2)(), (*)();
    func :12, ((**mem));
    int xx: 0-1, xx2: 1-1;
    int m: 1, n: 2;
    :0;
};
