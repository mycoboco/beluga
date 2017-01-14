/* -Wv --std=c90 */

auto int **xx;
extern register int *xx2;
register extern int *xx3;

int ((xx4));
enum { A } ((xx4));

int ((xx5)) = 0;
int ((xx5)) = 1;

extern int ((xx6));
static int ((xx6));

extern int ((xx7));
int ((xx7));
static int ((xx7));

static **xx8;
**xx8;

static struct tag ((xx9));

void ((xx10))(short, float);
void ((xx10))(aa, bb) char aa; double bb; {}

void ((xx11))(int, float, int *);
void ((xx11))(aa) {}

void ((x12))(int);
void ((x12))(aa, bb) {}

void xx13(int, register long double *()) {}

void ((xx14))(int *);
void ((xx14))(enum { B } (*(p))) {}

int **((xx15));
void ((xx16))(int **((x15))) {}
