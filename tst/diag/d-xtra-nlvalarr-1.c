void func(void)
{
    int i;
    struct tag {
        int w;
        int x[10], y:10;
        struct {
            int w;
            int x[10];
        } z;
    };
    extern struct tag f();

    f().x;
    f().x[0];
    f().x[1];
    &f().x;
    &f().x[0];
    &f().x[1];
    sizeof(f().x);
    sizeof(f().x[0]);
    sizeof(f().x[1]);
    f().x = 0;
    f().x[1] = 0;
    f().x++;
    f().x--;
    f().x = f().x;
    f().x < 0;
    f().x | 0;
    f().x == 0;
    f().x % 1;
    f().x+2 - f().x;
    f().x * 10;
    f().x / 10;
    +f().x;
    -f().x;
    ~f().x;
    !f().x;
    f().x();
    f().x->x;

    ((struct tag)1).x;
    ((struct tag)1).x[0];
    ((struct tag)1).x[1];
    &((struct tag)1).x;
    &((struct tag)1).x[0];
    &((struct tag)1).x[1];
    sizeof(((struct tag)1).x);
    sizeof(((struct tag)1).x[0]);
    sizeof(((struct tag)1).x[1]);
    ((struct tag)1).x = 0;
    ((struct tag)1).x[1] = 0;
    ((struct tag)1).x++;
    ((struct tag)1).x--;
    ((struct tag)1).x = ((struct tag)1).x;
    ((struct tag)1).x < 0;
    ((struct tag)1).x | 0;
    ((struct tag)1).x == 0;
    ((struct tag)1).x % 1;
    ((struct tag)1).x+2 - f()((struct tag)1).x;
    ((struct tag)1).x * 10;
    ((struct tag)1).x / 10;
    +((struct tag)1).x;
    -((struct tag)1).x;
    ~((struct tag)1).x;
    !((struct tag)1).x;
    ((struct tag)1).x();
    ((struct tag)1).x->x;

    ((struct tag)i).x;
    ((struct tag)i).x[0];
    ((struct tag)i).x[1];
    &((struct tag)i).x;
    &((struct tag)i).x[0];
    &((struct tag)i).x[1];
    sizeof(((struct tag)i).x);
    sizeof(((struct tag)i).x[0]);
    sizeof(((struct tag)i).x[1]);
    ((struct tag)i).x = 0;
    ((struct tag)i).x[1] = 0;
    ((struct tag)i).x++;
    ((struct tag)i).x--;
    ((struct tag)i).x = ((struct tag)i).x;
    ((struct tag)i).x < 0;
    ((struct tag)i).x | 0;
    ((struct tag)i).x == 0;
    ((struct tag)i).x % 1;
    ((struct tag)i).x+2 - ((struct tag)1).x;
    ((struct tag)i).x * 10;
    ((struct tag)i).x / 10;
    +((struct tag)i).x;
    -((struct tag)i).x;
    ~((struct tag)i).x;
    !((struct tag)i).x;
    ((struct tag)i).x();
    ((struct tag)i).x->x;

    ((int [10])1);
    ((int [10])1)[0];
    ((int [10])1)[1];
    &((int [10])1);
    &((int [10])1)[0];
    &((int [10])1)[1];
    sizeof(((int [10])1));
    sizeof(((int [10])1)[0]);
    sizeof(((int [10])1)[1]);
    ((int [10])1) = 0;
    ((int [10])1)[1] = 0;
    ((int [10])1)++;
    ((int [10])1)--;
    ((int [10])1) = ((int [10])1);
    ((int [10])1) < 0;
    ((int [10])1) | 0;
    ((int [10])1) == 0;
    ((int [10])1) % 1;
    ((int [10])1)+2 - ((int [10])1);
    ((int [10])1) * 10;
    ((int [10])1) / 10;
    +((int [10])1);
    -((int [10])1);
    ~((int [10])1);
    !((int [10])1);
    ((int [10])1)();
    ((int [10])1)->x;

    ((int [])1);
    ((int [])1)[0];
    ((int [])1)[1];
    &((int [])1);
    &((int [])1)[0];
    &((int [])1)[1];
    sizeof(((int [])1));
    sizeof(((int [])1)[0]);
    sizeof(((int [])1)[1]);
    ((int [])1) = 0;
    ((int [])1)[1] = 0;
    ((int [])1)++;
    ((int [])1)--;
    ((int [])1) = ((int [])1);
    ((int [])1) < 0;
    ((int [])1) | 0;
    ((int [])1) == 0;
    ((int [])1) % 1;
    ((int [])1)+2 - ((int [])1);
    ((int [])1) * 10;
    ((int [])1) / 10;
    +((int [])1);
    -((int [])1);
    ~((int [])1);
    !((int [])1);
    ((int [])1)();
    ((int [])1)->x;

    ((int [10])i);
    ((int [10])i)[0];
    ((int [10])i)[1];
    &((int [10])i);
    &((int [10])i)[0];
    &((int [10])i)[1];
    sizeof(((int [10])i));
    sizeof(((int [10])i)[0]);
    sizeof(((int [10])i)[1]);
    ((int [10])i) = 0;
    ((int [10])i)[1] = 0;
    ((int [10])i)++;
    ((int [10])i)--;
    ((int [10])i) = ((int [10])i);
    ((int [10])i) < 0;
    ((int [10])i) | 0;
    ((int [10])i) == 0;
    ((int [10])i) % 1;
    ((int [10])i)+2 - ((int [10])i);
    ((int [10])i) * 10;
    ((int [10])i) / 10;
    +((int [10])i);
    -((int [10])i);
    ~((int [10])i);
    !((int [10])i);
    ((int [10])i)();
    ((int [10])i)->x;

    ((int [])i);
    ((int [])i)[0];
    ((int [])i)[1];
    &((int [])i);
    &((int [])i)[0];
    &((int [])i)[1];
    sizeof(((int [])i));
    sizeof(((int [])i)[0]);
    sizeof(((int [])i)[1]);
    ((int [])i) = 0;
    ((int [])i)[1] = 0;
    ((int [])i)++;
    ((int [])i)--;
    ((int [])i) = ((int [])i);
    ((int [])i) < 0;
    ((int [])i) | 0;
    ((int [])i) == 0;
    ((int [])i) % 1;
    ((int [])i)+2 - ((int [])i);
    ((int [])i) * 10;
    ((int [])i) / 10;
    +((int [])i);
    -((int [])i);
    ~((int [])i);
    !((int [])i);
    ((int [])i)();
    ((int [])i)->x;
}