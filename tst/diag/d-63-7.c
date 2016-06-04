/* -W --std=c90 --parsable */

void f1(void) {
    x /* comment */ :
    ;
}

void f2(void) {
    x/* comment */:;
}

void f3(void) {
    x /* comment

    */ :;
}

void f4(void) {
    x /* comment
    */
    /* comment */ :
    ;
}

void f5(void) {
    x/**//**/:;
}

void g1(void) {
    x // comment
    :;
}

void g2(void) {
    x//
    :;
}

void g3(void) {
    x
    // comment
    :;
}

void g4(void) {
    x
    // comment
    // comment
    :;
}
