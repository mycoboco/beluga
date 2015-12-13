int a1[](void);
void f2(void) { int a2[1](void); a2; }
void f3(void) { int a3[1](void); int (**a)(void); a = a3; }
void a4[10];
struct tag a5[1];
int a6[0x7FFFFFFF/sizeof(int)+1];
int a7[0x7FFF/sizeof(int)+1];
extern void a8[10];
extern void a9[];
extern void a10[][10];
extern void a11[10][];
extern void a12[][];
int a13[];
int a14[10][];
int a15[][];
int a16[][10];
int a17[][][10];
int a18[][10][];
int a19[10][][];
int a20[][][];
