void f(void) {
    int x;
    break;    /* error */
    for (;;)
        break;
    while (0) {
        while (1)
            break;
        break;
    }
    switch(x) {
        case 1:
            while (1)
                break;
            break;
        case 2:
            continue;    /* error */
    }
    continue;    /* error */
    for (;1;)
        continue;
    while(1) {
        do continue; while(0);
        continue;
    }
}
