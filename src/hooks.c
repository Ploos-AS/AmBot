#include <stdio.h>

#include <dos/dos.h>
#include <proto/dos.h>

#include "events.h"
#include "hooks.h"
#include "rexx.h"

static const char *script_name(enum ambot_event_type type)
{
    switch (type) {
    case AMBOT_EVENT_PRIVMSG: return "ON_PRIVMSG.rexx";
    case AMBOT_EVENT_NOTICE: return "ON_NOTICE.rexx";
    case AMBOT_EVENT_JOIN: return "ON_JOIN.rexx";
    case AMBOT_EVENT_PART: return "ON_PART.rexx";
    case AMBOT_EVENT_QUIT: return "ON_QUIT.rexx";
    case AMBOT_EVENT_KICK: return "ON_KICK.rexx";
    case AMBOT_EVENT_TOPIC: return "ON_TOPIC.rexx";
    case AMBOT_EVENT_NICK: return "ON_NICK.rexx";
    case AMBOT_EVENT_CONNECT: return "ON_CONNECT.rexx";
    case AMBOT_EVENT_DISCONNECT: return "ON_DISCONNECT.rexx";
    default: return 0;
    }
}

static int script_exists(const char *path)
{
    BPTR lock = Lock((STRPTR)path, ACCESS_READ);
    if (lock == 0) return 0;
    UnLock(lock);
    return 1;
}

void ambot_hooks_init(struct ambot_hook_context *context,
                      struct ambot_rexx *rexx,
                      struct ambot_control *control)
{
    context->script_dir = AMBOT_HOOK_SCRIPT_DIR;
    context->rexx = rexx;
    context->control = control;
    context->invoked = 0;
    context->failed = 0;
}

void ambot_hooks_dispatch_event(const struct ambot_event *event, void *userdata)
{
    struct ambot_hook_context *context = (struct ambot_hook_context *)userdata;
    const char *script = script_name(event->type);
    char path[128];
    int written;
    int rc;

    if (context == 0 || script == 0) return;

    written = snprintf(path, sizeof(path), "%s/%s", context->script_dir, script);
    if (written <= 0 || written >= (int)sizeof(path)) return;
    if (!script_exists(path)) return;

    ++context->invoked;
    rc = ambot_rexx_run_script(context->rexx,
                               context->control,
                               path,
                               ambot_event_type_name(event->type),
                               event->nick,
                               event->target,
                               event->text);
    if (rc != 0) {
        ++context->failed;
        printf("AmBot: hook %s returned RC %d\n", script, rc);
    }
}
