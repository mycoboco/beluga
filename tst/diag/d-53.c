void f(int r) {
    int x, y, z, a[10], *p;

    r = 42 + x? y: z;
    f(y? x: z + 42);

    if (x? y: z == 0) f(0);
    if (x? y: z || 0) f(0);

    p = x? &y: &a[z];
}
