#include "weld.h"
#include "weld_utils.h"

#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_passed = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_passed++; printf("  ok   %s\n", msg); } \
    else      { g_failed++; printf("  FAIL %s\n", msg); } \
} while (0)

static void noop(const char *m) { (void)m; }

static void test_load_and_boot(void) {
    puts("test_load_and_boot");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    ASSERT(weld_load(c, "./modules/a.so") == 0, "load a");
    ASSERT(weld_load(c, "./modules/b.so") == 0, "load b");
    ASSERT(weld_load(c, "./modules/c.so") == 0, "load c");
    ASSERT(weld_module_count(c) == 3, "count == 3");
    ASSERT(weld_boot(c) == 0, "boot ok");
    weld_shutdown(c);
    weld_destroy_core(c);
}

static void test_missing_dep(void) {
    puts("test_missing_dep");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    weld_load(c, "./modules/bad.so");
    int rc = weld_boot(c);
    ASSERT(rc != 0, "boot fails on missing dep");
    weld_destroy_core(c);
}

static void test_cycle(void) {
    puts("test_cycle");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    weld_load(c, "./modules/cycle_x.so");
    weld_load(c, "./modules/cycle_y.so");
    int rc = weld_boot(c);
    ASSERT(rc != 0, "boot fails on cycle");
    weld_destroy_core(c);
}

static void test_utils_check(void) {
    puts("test_utils_check");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    weld_load(c, "./modules/a.so");
    weld_load(c, "./modules/b.so");
    weld_load(c, "./modules/bad.so");
    WeldDepReport r = weld_check_deps(c);
    ASSERT(r.total == 3, "total == 3");
    ASSERT(r.ok == 2, "ok == 2");
    ASSERT(r.missing == 1, "missing == 1");
    weld_destroy_core(c);
}

static void test_utils_list(void) {
    puts("test_utils_list");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    weld_load(c, "./modules/a.so");
    weld_load(c, "./modules/b.so");
    FILE *f = fopen("/tmp/weld_list.txt", "w");
    weld_list_modules(c, f);
    fclose(f);
    f = fopen("/tmp/weld_list.txt", "r");
    char buf[512] = {0};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    ASSERT(strstr(buf, "a") != NULL, "list contains a");
    ASSERT(strstr(buf, "b") != NULL, "list contains b");
    weld_destroy_core(c);
}

static void test_utils_tree(void) {
    puts("test_utils_tree");
    weld_set_error_handler(noop);
    WeldCore *c = weld_create();
    weld_load(c, "./modules/a.so");
    weld_load(c, "./modules/b.so");
    weld_load(c, "./modules/c.so");
    FILE *f = fopen("/tmp/weld_tree.txt", "w");
    weld_print_tree(c, f);
    fclose(f);
    f = fopen("/tmp/weld_tree.txt", "r");
    char buf[1024] = {0};
    fread(buf, 1, sizeof(buf) - 1, f);
    fclose(f);
    ASSERT(strstr(buf, "c") != NULL, "tree contains c (root)");
    ASSERT(strstr(buf, "a") != NULL, "tree contains a (leaf)");
    weld_destroy_core(c);
}
static void test_load_named(void) {
    puts("test_load_named");
    weld_set_error_handler(noop);
    weld_set_path("./modules");
    WeldCore *c = weld_create();
    ASSERT(weld_load_named(c, "a") == 0, "load_named resolves a.so");
    ASSERT(weld_module_count(c) == 1, "count == 1");
    weld_destroy_core(c);
    weld_set_path(NULL);
}
int main(void) {
    test_load_and_boot();
    test_missing_dep();
    test_cycle();
    test_utils_check();
    test_utils_list();
    test_utils_tree();
    test_load_named();
    printf("\n%d passed, %d failed\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
