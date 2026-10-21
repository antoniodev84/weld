#include "weld.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static int do_list(const char *path) {
    DIR *d = opendir(path);
    if (!d) { perror("opendir"); return -1; }
    struct dirent *e;
    while ((e = readdir(d))) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        printf("%s\n", e->d_name);
    }
    closedir(d);
    return 0;
}

static int do_copy(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    if (!in) { perror("fopen src"); return -1; }
    FILE *out = fopen(dst, "wb");
    if (!out) { perror("fopen dst"); fclose(in); return -1; }
    char buf[8192];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
        fwrite(buf, 1, n, out);
    fclose(in);
    fclose(out);
    return 0;
}

static int do_remove(const char *path) {
    if (remove(path) != 0) { perror("remove"); return -1; }
    return 0;
}

static void *fs_command(const char *cmd, void *arg) {
    if (strcmp(cmd, "list") == 0)
        return (void *)(long)do_list((const char *)arg);
    if (strcmp(cmd, "copy") == 0) {
        char **a = (char **)arg;
        return (void *)(long)do_copy(a[0], a[1]);
    }
    if (strcmp(cmd, "remove") == 0)
        return (void *)(long)do_remove((const char *)arg);
    return NULL;
}

static int  m_init(void)       { return 0; }
static void m_update(float dt) { (void)dt; }
static void m_destroy(void)    {}

static const char *deps[] = { "io" };

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
WeldModule *weld_module(void) {
    static WeldModule m = { "fs", deps, 1, m_init, m_update, m_destroy, fs_command };
    return &m;
}
