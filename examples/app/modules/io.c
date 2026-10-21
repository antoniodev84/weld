#include "weld.h"
#include <stdio.h>
#include <string.h>

static void *io_command(const char *cmd, void *arg) {
    if (strcmp(cmd, "info") == 0)  { printf("[INFO]  %s\n", (const char *)arg); return (void *)0; }
    if (strcmp(cmd, "warn") == 0)  { printf("[WARN]  %s\n", (const char *)arg); return (void *)0; }
    if (strcmp(cmd, "error") == 0) { fprintf(stderr, "[ERROR] %s\n", (const char *)arg); return (void *)0; }
    return NULL;
}

static int  m_init(void)       { return 0; }
static void m_update(float dt) { (void)dt; }
static void m_destroy(void)    {}

#ifdef _WIN32
__declspec(dllexport)
#else
__attribute__((visibility("default")))
#endif
WeldModule *weld_module(void) {
    static WeldModule m = { "io", NULL, 0, m_init, m_update, m_destroy, io_command };
    return &m;
}
