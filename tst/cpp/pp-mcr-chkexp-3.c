#define yy(o, s1, s2) OP_##o##s1##s2 = OP_##o + (s1 << 4) + s2

yy(CVFF, 2, 3),
