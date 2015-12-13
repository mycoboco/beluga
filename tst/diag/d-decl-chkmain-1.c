static void main() { }
int main() { return 0; }
int main(a, p) int a; char **p; { return 0; }
int main(int a, char **p) { return 0; }
int main(int a, char *p[]) { return 0; }
typedef int INT;
typedef char *PCHAR;
typedef void VOID;
INT main(INT a, PCHAR *p) { return 0; }
INT main(VOID) { return 0; }
/* warning triggered below */
void main(int a, char **p) { }
const int main(void) { return 0; }
int main(const int a, char **p) { return 0; }
int main(int a, const char **p) { return 0; }
int main(int a, char * const *p) { return 0; }
int main(int a, char ** const p) { return 0; }
int main(int a, char **p, ...) { return 0; }
