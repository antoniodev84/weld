#include "weld_utils.h"

#include <stdlib.h>
#include <string.h>

static WeldModule *weld_utils_find(WeldCore *core, const char *name) {
    for (size_t i = 0; i < weld_module_count(core); i++) {
        const char *n = weld_module_name(core, i);
        if (n && strcmp(n, name) == 0) {
            WeldCore *c = core;
            (void)c;
            return weld_get_module(core, i);
        }
    }
    return NULL;
}

WeldDepReport weld_check_deps(WeldCore *core) {
    WeldDepReport r = {0};
    if (!core) return r;

    size_t n = weld_module_count(core);
    for (size_t i = 0; i < n; i++) {
        WeldModule *m = weld_get_module(core, i);
        if (!m) continue;
        r.total++;
        int has_missing = 0;
        for (size_t d = 0; d < m->dep_count; d++) {
            if (!weld_utils_find(core, m->deps[d])) {
                has_missing = 1;
                break;
            }
        }
        if (has_missing) r.missing++;
        else r.ok++;
    }
    return r;
}

void weld_list_deps(WeldCore *core, const char *module, FILE *out) {
    if (!core || !module || !out) return;
    WeldModule *m = weld_utils_find(core, module);
    if (!m) { fprintf(out, "module '%s' not found\n", module); return; }
    if (m->dep_count == 0) {
        fprintf(out, "%s: (no dependencies)\n", m->name);
        return;
    }
    fprintf(out, "%s depends on:\n", m->name);
    for (size_t i = 0; i < m->dep_count; i++)
        fprintf(out, "  - %s\n", m->deps[i]);
}

void weld_list_modules(WeldCore *core, FILE *out) {
    if (!core || !out) return;
    size_t n = weld_module_count(core);
    if (n == 0) { fprintf(out, "(no modules loaded)\n"); return; }
    fprintf(out, "%-20s %-8s %s\n", "NAME", "DEPS", "STATUS");
    for (size_t i = 0; i < n; i++) {
        WeldModule *m = weld_get_module(core, i);
        if (!m) continue;
        const char *status = "ok";
        for (size_t d = 0; d < m->dep_count; d++) {
            if (!weld_utils_find(core, m->deps[d])) { status = "MISSING"; break; }
        }
        fprintf(out, "%-20s %-8zu %s\n", m->name, m->dep_count, status);
    }
}

static void weld_print_tree_rec(WeldCore *core, WeldModule *m, FILE *out, int depth) {
    for (int i = 0; i < depth; i++) fputs("  ", out);
    fputs(m->name, out);
    if (m->dep_count == 0) { fputs(" (leaf)\n", out); return; }
    fputc('\n', out);
    for (size_t i = 0; i < m->dep_count; i++) {
        WeldModule *d = weld_utils_find(core, m->deps[i]);
        for (int j = 0; j <= depth; j++) fputs("  ", out);
        if (!d) { fprintf(out, "- %s (MISSING)\n", m->deps[i]); continue; }
        fputc('-', out); fputc(' ', out);
        weld_print_tree_rec(core, d, out, depth + 1);
    }
}

void weld_print_tree(WeldCore *core, FILE *out) {
    if (!core || !out) return;
    size_t n = weld_module_count(core);
    for (size_t i = 0; i < n; i++) {
        WeldModule *m = weld_get_module(core, i);
        if (!m) continue;
        int is_dep = 0;
        for (size_t j = 0; j < n; j++) {
            WeldModule *o = weld_get_module(core, j);
            if (!o || o == m) continue;
            for (size_t d = 0; d < o->dep_count; d++)
                if (strcmp(o->deps[d], m->name) == 0) { is_dep = 1; break; }
            if (is_dep) break;
        }
        if (is_dep) continue;
        weld_print_tree_rec(core, m, out, 0);
    }
}
