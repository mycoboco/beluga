const char *p1 = "test\123z";
const char *p2 = "test\12z";
const char *p3 = "test\5678z";    /* warning */
const char *p4 = "test\0123z";    /* warning */
const char *p5 = "test\6789z";    /* warning */
