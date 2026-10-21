#include "weld.h"
static int  i(void){return 0;}
static void u(float d){(void)d;}
static void x(void){}
static const char *deps[] = { "a" };
__attribute__((visibility("default")))
WeldModule *weld_module(void) {
    static WeldModule m = { "b", deps, 1, i, u, x, NULL };
    return &m;
}
