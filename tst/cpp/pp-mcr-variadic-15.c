/* -WXv */
/* from https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61613 */

const char* print(int x, const char* s) { puts(s); return "result"; }
const char* prinz(int x, const char* s) { puts(s); return "result"; }

#define print(x,...) (print(1,"macro"),print(1, ##__VA_ARGS__))
#define prinz(x,...) (prinz(1,"macro"),prinz(1, __VA_ARGS__))

int main() {
    print(1,print(1,"world"));
    puts("");
    prinz(1,prinz(1,"world"));
}
