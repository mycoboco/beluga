void f1(void) { goto lab1; }
void f2(void) { lab2: f1(); goto lab2; }
void f3(void) { goto lab3; f1(); lab3:; }
void f4(void) { switch(1) { case 0: defualt: return; } }
void f5(void) { lab4: return; }
