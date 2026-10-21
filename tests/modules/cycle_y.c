#include "weld.h"
static int  i(void){return 0;}
static void u(float d){(void)d;}
static void x(void){}
static const char *deps[] = { "cycle_x" };
__attribute__((visibility("default")))
WeldModule *weld_module(void) {
    static WeldModule m = { "cycle_y", deps, 1, i, u, x, NULL };
    return &m;
}
