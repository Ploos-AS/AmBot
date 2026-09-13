#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <proto/dos.h>

#include "modules.h"

static int has_rexx_suffix(const char *name)
{
    unsigned int n = (unsigned int)strlen(name);
    if (n < 5) return 0;
    return strcmp(name + n - 5, ".rexx") == 0 || strcmp(name + n - 5, ".REXX") == 0;
}

static void copy_text(char *dst, unsigned int size, const char *src)
{
    unsigned int n;
    if (size == 0) return;
    n = (unsigned int)strlen(src);
    if (n >= size) n = size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

void ambot_modules_init(struct ambot_modules *modules)
{
    memset(modules, 0, sizeof(*modules));
}

int ambot_modules_discover(struct ambot_modules *modules, const char *directory)
{
    BPTR lock;
    struct FileInfoBlock fib;

    ambot_modules_init(modules);
    if (directory == 0 || directory[0] == '\0') return 0;

    lock = Lock((STRPTR)directory, ACCESS_READ);
    if (lock == 0) return -1;
    if (!Examine(lock, &fib)) {
        UnLock(lock);
        return -1;
    }

    while (ExNext(lock, &fib)) {
        char path[AMBOT_MODULE_PATH_MAX + 1];
        int written;
        if (fib.fib_DirEntryType >= 0 || !has_rexx_suffix(fib.fib_FileName)) continue;
        if (modules->count >= AMBOT_MODULES_MAX) {
            ++modules->skipped;
            continue;
        }

        written = snprintf(path, sizeof(path), "%s/%s", directory, fib.fib_FileName);
        if (written <= 0 || written >= (int)sizeof(path)) {
            ++modules->skipped;
            continue;
        }
        copy_text(modules->items[modules->count].name,
                  sizeof(modules->items[modules->count].name), fib.fib_FileName);
        copy_text(modules->items[modules->count].path,
                  sizeof(modules->items[modules->count].path), path);
        modules->items[modules->count].state[0] = '\0';
        ++modules->count;
    }

    UnLock(lock);
    return 0;
}

struct ambot_module *ambot_modules_find(struct ambot_modules *modules, const char *name)
{
    unsigned int i;
    for (i = 0; i < modules->count; ++i) {
        if (strcmp(modules->items[i].name, name) == 0) return &modules->items[i];
    }
    return 0;
}

int ambot_module_set_state(struct ambot_module *module, const char *state)
{
    if (module == 0 || state == 0) return -1;
    if (strlen(state) > AMBOT_MODULE_STATE_MAX) return -1;
    copy_text(module->state, sizeof(module->state), state);
    return 0;
}
