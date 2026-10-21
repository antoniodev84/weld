# weld

A tiny module loader for C. ~500 lines, zero dependencies, builds in under 1 second.

Weld does one thing: it loads modules, resolves their dependencies, and calls their lifecycle functions in the right order. It doesn't know what your modules do — that's the point.

## Why

Big C projects turn into spaghetti: circular header dependencies, hardcoded init order, everything linked together. Weld lets you split your code into modules, declare what each one needs, and let the core figure out the rest.

## What it does

- Loads `.so` / `.dll` modules at runtime (`dlopen` / `LoadLibrary`)
- Resolves dependencies with topological sort
- Detects cycles and missing dependencies
- Fixed lifecycle: `init` → `update` → `destroy` (reverse order)
- String-based dispatch so `main` never includes module headers

## What it doesn't do

No build system. No codegen. No macros. No opinions about your architecture.

## Build

```sh
make
make install
```

## Module contract

Every module exports one symbol, weld_module(), that returns a WeldModule:

```c
#include "weld.h"
#include <string.h>

static void *my_command(const char *cmd, void *arg) {
    if (strcmp(cmd, "hello") == 0) {
        puts("hello from my module");
        return NULL;
    }
    return NULL;
}

static int  m_init(void)       { return 0; }
static void m_update(float dt) { (void)dt; }
static void m_destroy(void)    {}

__attribute__((visibility("default")))
WeldModule *weld_module(void) {
    static WeldModule m = {
        "my",          /* name       */
        NULL, 0,       /* deps       */
        m_init,
        m_update,
        m_destroy,
        my_command     /* dispatch   */
    };
    return &m;
}
```

## Build it as a shared library:

```sh
bash build.sh
```

Loading from your app

```c
#include "weld.h"

int main(void) {
    WeldCore *core = weld_create();

    weld_load(core, "./my.so");
    weld_boot(core);

    weld_call(core, "my", "hello", NULL);

    weld_shutdown(core);
    weld_destroy_core(core);
    return 0;
}
```

The main never includes my.h. It only talks to modules through weld_call.

## Example

See examples/app — a small file manager CLI built from two modules (io, fs) loaded at runtime.

```sh
cd examples/app
make
./filecli modules
./filecli list .
./filecli copy main.c /tmp/x.c
./filecli remove /tmp/x.c
```

Output:

```
io
fs
[INFO]  listing directory
Makefile
main.c
modules
...
```

## Dependencies in action

A module declares what it needs:

```c
static const char *deps[] = { "io" };
```

Weld sorts them and inits io before fs — no matter the load order. Cycles and missing deps are reported as errors, not crashes.

## Platform

· Linux (dlopen, link with -ldl)
· Windows (LoadLibrary, MinGW)

## License

MIT
