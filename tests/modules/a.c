#include "weld.h"
static int  i(void){return 0;}
static void u(float d){(void)d;}
static void x(void){}
__attribute__((visibility("default")))
WeldModule *weld_module(void) {
    static WeldModule m = { "a", NULL, 0, i, u, x, NULL };
    return &m;
}
