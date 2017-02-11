typedef int myint;
typedef myint aint[10];
typedef volatile int vint;
typedef vint avint[10];
typedef struct { vint x; } str;

vint g(void);

void f(void)
{
    vint Vint;
    aint Aint;
    avint Avint;
    str Str, *pstr;

    (Aint[0]) = Str;
    (Avint[0]) = Str;
    g() * Str;
    Str.x = Str;
    pstr->x = Str;
    Aint[0]++ * Str;
    Avint[0]++ * Str;

    ++Aint[0] * Str;
    ++Avint[0] * Str;
    &Aint[0] * Str;
    &Avint[0] * Str;
    *Aint = Str;
    *Avint = Str;
    +Vint * Str;
    -Vint * Str;
    ~Vint * Str;
    !Vint * Str;

    (Vint + Vint) * Str;
    (pstr + Vint) * Str;
    (Vint * Vint) * Str;
    (Vint % Vint) * Str;
    (Vint << Vint) * Str;
    (Vint < Vint) * Str;
    (Vint == Vint) * Str;
    (Vint & Vint) * Str;
    (Vint && Vint) * Str;
    (Vint = 0) * Str;
}
