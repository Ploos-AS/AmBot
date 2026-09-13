#ifndef AMBOT_HOOKS_H
#define AMBOT_HOOKS_H

#include "events.h"

#define AMBOT_HOOK_SCRIPT_DIR "REXX:AmBot"

struct ambot_hook_context {
    const char *script_dir;
    unsigned long invoked;
    unsigned long failed;
};

void ambot_hooks_init(struct ambot_hook_context *context);
void ambot_hooks_dispatch_event(const struct ambot_event *event, void *userdata);

#endif
