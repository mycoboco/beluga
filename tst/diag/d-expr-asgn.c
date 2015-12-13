void f1(void) { int a, b; a = b; }
void f2(void) { int a, b, c; a = b = c; }
void f3(void) { int a, b, c; (a = b) = c; }    /* error */
void f4(void)  { int a, b, c; a += b; a += b += c; }
void f5(void)  { int a, b, c; a -= b; a -= b -= c; }
void f6(void)  { int a, b, c; a *= b; a *= b *= c; }
void f7(void)  { int a, b, c; a /= b; a /= b /= c; }
void f8(void)  { int a, b, c; a %= b; a %= b %= c; }
void f9(void)  { int a, b, c; a &= b; a &= b &= c; }
void f10(void) { int a, b, c; a |= b; a |= b |= c; }
void f11(void) { int a, b, c; a ^= b; a ^= b ^= c; }
void f12(void) { int a, b, c; a <<= b; a <<= b <<= c; }
void f13(void) { int a, b, c; a >>= b; a >>= b >>= c; }
