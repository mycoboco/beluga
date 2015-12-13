enum e1 { A };
enum e2 { B };

void f1(enum e1);
void f1(x) enum e2 x; {}

void f2(enum e1);
void f2(x) enum e1 x; {}

void f3(enum e1);
void f3(x) int x; {}

void f4(int);
void f4(x) enum e1 x; {}
