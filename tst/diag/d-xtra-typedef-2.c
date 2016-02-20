typedef double A, A2;
typedef float *B;

int f3(short (A, B));
int f3(short (double, float *));

int f6(short A, B);
int f6(short A, float *);

int f9(short (A), (B));

int f11(void) {
    int (A)(int, B);
    int A(int, float *);
}

int (A)(int a, B b) { return 0; }
int A2(void) { return 0; }

int f20(short (A)) { return 0; }

int f22(short A, (B));

int f24(short (B), int (c));
int f24(short (float *), int c);
