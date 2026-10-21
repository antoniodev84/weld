#ifndef WELD_H
#define WELD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct WeldModule WeldModule;

typedef int    (*weld_init_fn)(void);
typedef void   (*weld_update_fn)(float dt);
typedef void   (*weld_destroy_fn)(void);
typedef void * (*weld_command_fn)(const char *cmd, void *arg);

struct WeldModule {
    const char *name;
    const char **deps;
    size_t dep_count;
    weld_init_fn init;
    weld_update_fn update;
    weld_destroy_fn destroy;
    weld_command_fn command;
};

typedef struct WeldCore WeldCore;

WeldCore *weld_create(void);
void      weld_destroy_core(WeldCore *core);

int weld_load(WeldCore *core, const char *path);

int  weld_boot(WeldCore *core);
void weld_update(WeldCore *core, float dt);
void weld_shutdown(WeldCore *core);

void *weld_call(WeldCore *core, const char *module, const char *cmd, void *arg);

size_t weld_module_count(const WeldCore *core);
const char *weld_module_name(const WeldCore *core, size_t index);

void weld_set_error_handler(void (*fn)(const char *msg));

#ifdef __cplusplus
}
#endif

#endif
