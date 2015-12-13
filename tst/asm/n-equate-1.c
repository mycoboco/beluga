int dlist(int);
void g(int, int);

int main(void)
{
    int c;

    g(c=dlist(0), dlist(c? 0: 1));
}
