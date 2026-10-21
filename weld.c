#include "weld.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
typedef HMODULE weld_handle;
static weld_handle weld_open(const char *p) { return LoadLibraryA(p); }
static void *weld_sym(weld_handle h, const char *s) { return (void *)GetProcAddress(h, s); }
static void weld_close(weld_handle h) { FreeLibrary(h); }
static const char *weld_err(void) { return "LoadLibrary failed"; }
#else
#include <dlfcn.h>
typedef void *weld_handle;
static weld_handle weld_open(const char *p) { return dlopen(p, RTLD_NOW | RTLD_LOCAL); }
static void *weld_sym(weld_handle h, const char *s) { return dlsym(h, s); }
static void weld_close(weld_handle h) { dlclose(h); }
static const char *weld_err(void) { return dlerror(); }
#endif

#ifdef _WIN32
#define WELD_PATH_SEP ';'
#define WELD_LIB_EXT ".dll"
#else
#define WELD_PATH_SEP ':'
#define WELD_LIB_EXT ".so"
#endif
typedef struct {
    WeldModule *mod;
    weld_handle handle;
    int state;
} weld_entry;

struct WeldCore {
    weld_entry *entries;
    size_t count;
    size_t cap;
    int booted;
};

static void (*g_error_fn)(const char *msg) = NULL;

void weld_set_error_handler(void (*fn)(const char *msg)) {
    g_error_fn = fn;
}

static void weld_error(const char *msg) {
    if (g_error_fn) g_error_fn(msg);
    else fprintf(stderr, "[weld] %s\n", msg);
}

WeldCore *weld_create(void) {
    return (WeldCore *)calloc(1, sizeof(WeldCore));
}

static void weld_unload_all(WeldCore *core) {
    for (size_t i = 0; i < core->count; i++) {
        if (core->entries[i].handle) weld_close(core->entries[i].handle);
        core->entries[i].handle = NULL;
    }
}

void weld_destroy_core(WeldCore *core) {
    if (!core) return;
    if (core->booted) weld_shutdown(core);
    weld_unload_all(core);
    free(core->entries);
    free(core);
}

int weld_load(WeldCore *core, const char *path) {
    if (!core || !path) return -1;

    weld_handle h = weld_open(path);
    if (!h) { weld_error(weld_err()); return -2; }

    WeldModule *(*get_mod)(void) = (WeldModule *(*)(void))weld_sym(h, "weld_module");
    if (!get_mod) {
        weld_error("symbol 'weld_module' not found");
        weld_close(h);
        return -3;
    }

    WeldModule *m = get_mod();
    if (!m || !m->name) {
        weld_error("invalid module descriptor");
        weld_close(h);
        return -4;
    }

    if (core->count == core->cap) {
        size_t ncap = core->cap ? core->cap * 2 : 8;
        weld_entry *ne = (weld_entry *)realloc(core->entries, ncap * sizeof(weld_entry));
        if (!ne) { weld_close(h); return -5; }
        core->entries = ne;
        core->cap = ncap;
    }

    core->entries[core->count].mod = m;
    core->entries[core->count].handle = h;
    core->entries[core->count].state = 0;
    core->count++;
    return 0;
}

static weld_entry *weld_find(WeldCore *core, const char *name) {
    for (size_t i = 0; i < core->count; i++)
        if (strcmp(core->entries[i].mod->name, name) == 0)
            return &core->entries[i];
    return NULL;
}

static int weld_visit(WeldCore *core, weld_entry *e, int depth) {
    if (e->state == 2) return 0;
    if (e->state == 1) { weld_error("cyclic dependency detected"); return -1; }
    if (depth > 256) { weld_error("dependency chain too deep"); return -1; }

    e->state = 1;
    for (size_t i = 0; i < e->mod->dep_count; i++) {
        weld_entry *dep = weld_find(core, e->mod->deps[i]);
        if (!dep) {
            char buf[256];
            snprintf(buf, sizeof(buf), "missing dependency '%s' for '%s'",
                     e->mod->deps[i], e->mod->name);
            weld_error(buf);
            return -2;
        }
        if (weld_visit(core, dep, depth + 1) != 0) return -3;
    }

    if (e->mod->init && e->mod->init() != 0) {
        char buf[256];
        snprintf(buf, sizeof(buf), "init failed for '%s'", e->mod->name);
        weld_error(buf);
        return -4;
    }
    e->state = 2;
    return 0;
}

int weld_boot(WeldCore *core) {
    if (!core || core->booted) return -1;
    for (size_t i = 0; i < core->count; i++)
        if (weld_visit(core, &core->entries[i], 0) != 0) return -2;
    core->booted = 1;
    return 0;
}

void weld_update(WeldCore *core, float dt) {
    if (!core || !core->booted) return;
    for (size_t i = 0; i < core->count; i++) {
        weld_entry *e = &core->entries[i];
        if (e->state == 2 && e->mod->update)
            e->mod->update(dt);
    }
}

void weld_shutdown(WeldCore *core) {
    if (!core || !core->booted) return;
    for (size_t i = core->count; i > 0; i--) {
        weld_entry *e = &core->entries[i - 1];
        if (e->state == 2 && e->mod->destroy)
            e->mod->destroy();
        e->state = 0;
    }
    core->booted = 0;
}

void *weld_call(WeldCore *core, const char *module, const char *cmd, void *arg) {
    if (!core || !core->booted) return NULL;
    weld_entry *e = weld_find(core, module);
    if (!e || !e->mod->command) return NULL;
    return e->mod->command(cmd, arg);
}

size_t weld_module_count(const WeldCore *core) {
    return core ? core->count : 0;
}

const char *weld_module_name(const WeldCore *core, size_t index) {
    if (!core || index >= core->count) return NULL;
    return core->entries[index].mod->name;
}
WeldModule *weld_get_module(WeldCore *core, size_t index) {
    if (!core || index >= core->count) return NULL;
    return core->entries[index].mod;
}
static char g_weld_path[4096];
static int  g_weld_path_init = 0;

const char *weld_get_path(void) {
    if (!g_weld_path_init) {
        const char *env = getenv("WELD_PATH");
        if (env && *env) {
            snprintf(g_weld_path, sizeof(g_weld_path), "%s", env);
        } else {
#ifdef _WIN32
            const char *home = getenv("USERPROFILE");
#else
            const char *home = getenv("HOME");
#endif
            if (home)
                snprintf(g_weld_path, sizeof(g_weld_path),
                         "%s/.local/lib/weld%c."
                         , home, WELD_PATH_SEP);
            else
                g_weld_path[0] = '\0';
        }
        g_weld_path_init = 1;
    }
    return g_weld_path;
}

void weld_set_path(const char *path) {
    if (!path) { g_weld_path[0] = '\0'; g_weld_path_init = 1; return; }
    snprintf(g_weld_path, sizeof(g_weld_path), "%s", path);
    g_weld_path_init = 1;
}

static int weld_file_exists(const char *p) {
    FILE *f = fopen(p, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

int weld_load_named(WeldCore *core, const char *name) {
    if (!core || !name) return -1;

    if (strchr(name, '/') || strchr(name, '\\') || strchr(name, '.'))
        return weld_load(core, name);

    const char *paths = weld_get_path();
    if (!paths || !*paths) return weld_load(core, name);

    char candidate[4096];
    const char *start = paths;

    while (*start) {
        const char *end = start;
        while (*end && *end != WELD_PATH_SEP) end++;
        size_t dirlen = (size_t)(end - start);

        if (dirlen > 0 && dirlen < sizeof(candidate) - 256) {
            snprintf(candidate, sizeof(candidate),
                     "%.*s/%s%s", (int)dirlen, start, name, WELD_LIB_EXT);
            if (weld_file_exists(candidate))
                return weld_load(core, candidate);
        }

        if (*end == WELD_PATH_SEP) end++;
        start = end;
    }

    return weld_load(core, name);
}
