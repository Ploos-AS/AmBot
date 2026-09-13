#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "commands.h"
#include "config.h"
#include "events.h"
#include "hooks.h"
#include "irc.h"
#include "modules.h"
#include "net.h"
#include "operations.h"
#include "rexx.h"
#include "session.h"

#define AMBOT_RECONNECT_MAX_SECONDS 30UL
#define AMBOT_RECV_BUFFER 512

struct line_context {
    int sock;
    struct ambot_event_queue *events;
};

struct dispatch_context {
    struct ambot_command_context *commands;
    struct ambot_hook_context *hooks;
};

static int ctrl_c_requested(void)
{
    return (SetSignal(0L, 0L) & SIGBREAKF_CTRL_C) != 0;
}

static void dispatch_event(const struct ambot_event *event, void *userdata)
{
    struct dispatch_context *context = (struct dispatch_context *)userdata;
    ambot_commands_handle_event(event, context->commands);
    ambot_hooks_dispatch_event(event, context->hooks);
}

static void emit_lifecycle(struct ambot_hook_context *hooks,
                           enum ambot_event_type type,
                           const struct ambot_config *config,
                           const char *reason)
{
    struct ambot_event event;
    memset(&event, 0, sizeof(event));
    event.type = type;
    snprintf(event.nick, sizeof(event.nick), "%s", config->nick);
    snprintf(event.target, sizeof(event.target), "%s", config->host);
    snprintf(event.text, sizeof(event.text), "%s", reason != 0 ? reason : "");
    ambot_hooks_dispatch_event(&event, hooks);
}

static void handle_line(const char *line, void *userdata)
{
    struct line_context *context = (struct line_context *)userdata;
    struct ambot_event event;

    printf("< %s\n", line);

    if (strncmp(line, "PING ", 5) == 0) {
        char response[AMBOT_IRC_LINE_MAX + 1];
        int written = snprintf(response, sizeof(response), "PONG %s", line + 5);

        if (written > 0 && written < (int)sizeof(response)) {
            printf("> %s\n", response);
            (void)ambot_irc_send_line(context->sock, response);
        }
        return;
    }

    if (ambot_event_from_irc_line(line, &event)) {
        if (ambot_event_queue_push(context->events, &event) != 0)
            printf("AmBot: event queue full; dropped=%lu\n", context->events->dropped);
    }
}

static void process_reload(struct ambot_control *control,
                           struct ambot_config *config,
                           struct ambot_modules *modules,
                           struct ambot_hook_context *hooks)
{
    if (!control->reload_requested) return;
    control->reload_requested = 0;

    if (config->path[0] == '\0') {
        puts("AmBot: reload requested but no config file is active");
        return;
    }
    if (ambot_config_load(config, config->path) != 0) {
        puts("AmBot: config reload failed");
        return;
    }

    hooks->script_dir = config->hook_dir;
    (void)ambot_modules_discover(modules, config->module_dir);
    printf("AmBot: config reloaded; channels=%u modules=%u skipped=%lu\n",
           (unsigned int)config->channel_count,
           (unsigned int)modules->count,
           modules->skipped);
}

static int run_connected_session(struct ambot_config *config,
                                 int sock,
                                 struct ambot_rexx *rexx,
                                 struct ambot_control *control,
                                 struct ambot_hook_context *hooks,
                                 struct ambot_modules *modules)
{
    struct ambot_irc_framer framer;
    struct ambot_event_queue events;
    struct ambot_command_context commands;
    struct dispatch_context dispatch;
    struct line_context context;
    char buffer[AMBOT_RECV_BUFFER];
    unsigned int i;

    context.sock = sock;
    context.events = &events;
    commands.sock = sock;
    commands.bot_nick = config->nick;
    commands.owner_nick = config->owner;
    dispatch.commands = &commands;
    dispatch.hooks = hooks;

    ambot_irc_framer_init(&framer);
    ambot_event_queue_init(&events);
    ambot_control_set_socket(control, sock);
    control->disconnect_requested = 0;

    if (ambot_irc_send_registration(sock, config->nick, config->user,
                                    config->pass[0] != '\0' ? config->pass : 0) != 0) {
        ambot_control_set_socket(control, -1);
        return -1;
    }

    for (i = 0; i < config->channel_count; ++i) {
        char line[AMBOT_IRC_LINE_MAX + 1];
        int written = snprintf(line, sizeof(line), "JOIN %s", config->channels[i]);
        if (written > 0 && written < (int)sizeof(line))
            (void)ambot_irc_send_line(sock, line);
    }

    while (!ctrl_c_requested() && !control->quit_requested &&
           !control->disconnect_requested) {
        unsigned long signals = 0;
        unsigned long wait_mask = ambot_rexx_signal_mask(rexx) | SIGBREAKF_CTRL_C;
        int readable = ambot_net_wait(sock, wait_mask, &signals);

        if ((signals & ambot_rexx_signal_mask(rexx)) != 0) {
            ambot_rexx_process(rexx, control);
            process_reload(control, config, modules, hooks);
        }
        if ((signals & SIGBREAKF_CTRL_C) != 0) {
            control->quit_requested = 1;
            break;
        }
        if (control->quit_requested || control->disconnect_requested) break;
        if (readable < 0) {
            ambot_control_set_socket(control, -1);
            return -1;
        }
        if (readable > 0) {
            int received = ambot_net_recv(sock, buffer, sizeof(buffer));
            if (received <= 0) {
                ambot_control_set_socket(control, -1);
                return -1;
            }
            ambot_irc_framer_feed(&framer, buffer, (unsigned int)received,
                                  handle_line, &context);
            ambot_event_dispatch_pending(&events, dispatch_event, &dispatch);
        }
    }

    ambot_control_set_socket(control, -1);
    return 0;
}

static void reconnect_delay(unsigned long seconds,
                            struct ambot_rexx *rexx,
                            struct ambot_control *control,
                            struct ambot_config *config,
                            struct ambot_modules *modules,
                            struct ambot_hook_context *hooks)
{
    unsigned long ticks = seconds * 50UL;
    while (ticks > 0 && control->connect_requested &&
           !control->quit_requested && !ctrl_c_requested()) {
        ambot_rexx_process(rexx, control);
        process_reload(control, config, modules, hooks);
        if (!control->connect_requested || control->quit_requested) break;
        Delay(ticks > 5 ? 5 : ticks);
        ticks = ticks > 5 ? ticks - 5 : 0;
    }
}

int ambot_session_run(const struct ambot_session_config *startup)
{
    struct ambot_rexx rexx;
    struct ambot_control control;
    struct ambot_hook_context hooks;
    struct ambot_config config;
    struct ambot_modules modules;
    unsigned long backoff = 1;

    ambot_config_seed(&config, startup->host, startup->port, startup->nick,
                      startup->user, startup->pass, startup->owner_nick);
    if (startup->config_path != 0 && startup->config_path[0] != '\0') {
        if (ambot_config_load(&config, startup->config_path) != 0) {
            puts("AmBot: unable to load config file");
            return 10;
        }
    }
    if (config.host[0] == '\0' || config.nick[0] == '\0') {
        puts("AmBot: config requires HOST and NICK");
        return 10;
    }

    ambot_modules_init(&modules);
    (void)ambot_modules_discover(&modules, config.module_dir);
    printf("AmBot: modules discovered=%u skipped=%lu\n",
           (unsigned int)modules.count, modules.skipped);

    ambot_control_init(&control);
    if (ambot_rexx_open(&rexx) != 0) {
        puts("AmBot: unable to create ARexx port AMBOT");
        return 20;
    }
    puts("AmBot: ARexx port AMBOT ready");
    ambot_hooks_init(&hooks, &rexx, &control);
    hooks.script_dir = config.hook_dir;

    if (ambot_net_open() != 0) {
        puts("AmBot: unable to open bsdsocket.library");
        ambot_rexx_close(&rexx);
        return 20;
    }

    while (!ctrl_c_requested() && !control.quit_requested) {
        int sock;
        int session_rc;

        ambot_rexx_process(&rexx, &control);
        process_reload(&control, &config, &modules, &hooks);
        if (!control.connect_requested) {
            Delay(5);
            continue;
        }

        printf("AmBot: connecting to %s:%u\n", config.host, (unsigned int)config.port);
        sock = ambot_net_connect_ipv4(config.host, config.port);

        if (sock >= 0) {
            puts("AmBot: connected");
            backoff = 1;
            ambot_control_set_socket(&control, sock);
            emit_lifecycle(&hooks, AMBOT_EVENT_CONNECT, &config, "connected");
            session_rc = run_connected_session(&config, sock, &rexx, &control, &hooks, &modules);
            ambot_net_close_socket(sock);
            ambot_control_set_socket(&control, -1);

            if (control.disconnect_requested)
                emit_lifecycle(&hooks, AMBOT_EVENT_DISCONNECT, &config, "requested");
            else if (control.quit_requested || ctrl_c_requested())
                emit_lifecycle(&hooks, AMBOT_EVENT_DISCONNECT, &config, "stopped");
            else if (session_rc != 0)
                emit_lifecycle(&hooks, AMBOT_EVENT_DISCONNECT, &config, "connection-lost");
            else
                emit_lifecycle(&hooks, AMBOT_EVENT_DISCONNECT, &config, "disconnected");

            if (control.quit_requested || ctrl_c_requested()) break;
            if (control.disconnect_requested) {
                control.disconnect_requested = 0;
                control.connect_requested = 0;
                puts("AmBot: disconnected by control request");
                continue;
            }
            control.connect_requested = 1;
            puts("AmBot: connection lost");
        } else {
            puts("AmBot: connection failed");
        }

        if (control.connect_requested) {
            printf("AmBot: reconnecting in %lu second(s)\n", backoff);
            reconnect_delay(backoff, &rexx, &control, &config, &modules, &hooks);
            if (backoff < AMBOT_RECONNECT_MAX_SECONDS) {
                backoff *= 2;
                if (backoff > AMBOT_RECONNECT_MAX_SECONDS)
                    backoff = AMBOT_RECONNECT_MAX_SECONDS;
            }
        }
    }

    printf("AmBot: hooks invoked=%lu failed=%lu modules=%u\n",
           hooks.invoked, hooks.failed, (unsigned int)modules.count);
    ambot_control_set_socket(&control, -1);
    ambot_net_close();
    ambot_rexx_close(&rexx);
    puts("AmBot: stopped");
    return 0;
}
