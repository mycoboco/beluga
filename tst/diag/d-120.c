/* -Wv -w --std=c90 */

void foo(int);

#define call_foo foo()
#define decl_bar int bar()

void fred(void)
{
    call_foo;
    decl_bar;
}
