#include "weld.h"

#include <stdio.h>
#include <string.h>

static void on_err(const char *msg) { fprintf(stderr, "[weld] %s\n", msg); }

static void usage(void) {
    puts("usage:");
    puts("  filecli list <dir>");
    puts("  filecli copy <src> <dst>");
    puts("  filecli remove <path>");
    puts("  filecli modules");
}

int main(int argc, char **argv) {
    weld_set_error_handler(on_err);

    WeldCore *core = weld_create();
    if (weld_load_named(core, "io") != 0) { weld_destroy_core(core); return 1; }
    if (weld_load_named(core, "fs") != 0) { weld_destroy_core(core); return 1; }
    if (weld_boot(core) != 0) { weld_destroy_core(core); return 1; }

    if (argc < 2) { usage(); weld_shutdown(core); weld_destroy_core(core); return 1; }

    const char *cmd = argv[1];

    if (strcmp(cmd, "list") == 0 && argc == 3) {
        weld_call(core, "io", "info", (void *)"listing directory");
        weld_call(core, "fs", "list", argv[2]);
    } else if (strcmp(cmd, "copy") == 0 && argc == 4) {
        char *a[2] = { argv[2], argv[3] };
        weld_call(core, "io", "info", (void *)"copying file");
        weld_call(core, "fs", "copy", a);
    } else if (strcmp(cmd, "remove") == 0 && argc == 3) {
        weld_call(core, "io", "info", (void *)"removing file");
        weld_call(core, "fs", "remove", argv[2]);
    } else if (strcmp(cmd, "modules") == 0) {
        for (size_t i = 0; i < weld_module_count(core); i++)
            printf("%s\n", weld_module_name(core, i));
    } else {
        usage();
        weld_shutdown(core);
        weld_destroy_core(core);
        return 1;
    }

    weld_shutdown(core);
    weld_destroy_core(core);
    return 0;
}
