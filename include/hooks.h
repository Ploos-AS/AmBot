#ifndef AMBOT_HOOKS_H
#define AMBOT_HOOKS_H

#include "events.h"

#define AMBOT_HOOK_SCRIPT_DIR "REXX:AmBot"

struct ambot_rexx;
struct ambot_control;

struct ambot_hook_context {
    const char *script_dir;
    struct ambot_rexx *rexx;
    struct ambot_control *control;
    unsigned long invoked;
    unsigned long failed;
};

void ambot_hooks_init(struct ambot_hook_context *context,
                      struct ambot_rexx *rexx,
                      struct ambot_control *control);
void ambot_hooks_dispatch_event(const struct ambot_event *event, void *userdata);

#endif
