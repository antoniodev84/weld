#ifndef WELD_UTILS_H
#define WELD_UTILS_H

#include "weld.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    size_t total;
    size_t ok;
    size_t missing;
    size_t cyclic;
} WeldDepReport;

WeldDepReport weld_check_deps(WeldCore *core);

void weld_list_deps(WeldCore *core, const char *module, FILE *out);

void weld_list_modules(WeldCore *core, FILE *out);

void weld_print_tree(WeldCore *core, FILE *out);

#ifdef __cplusplus
}
#endif

#endif
