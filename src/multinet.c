#include <stdio.h>
#include <string.h>
#include <time.h>

#include <proto/exec.h>

#include "commands.h"
#include "events.h"
#include "hooks.h"
#include "irc.h"
#include "modernirc.h"
#include "modules.h"
#include "multinet.h"
#include "net.h"
#include "networks.h"
#include "operations.h"
#include "rexx.h"

#define AMBOT_MULTINET_RECV_BUFFER 512

struct network_runtime {
    struct ambot_network_config *config;
    int sock;
    struct ambot_irc_framer framer;
    struct ambot_event_queue events;
    struct ambot_command_context commands;
    struct ambot_hook_context hooks;
    struct ambot_modules modules;
    struct ambot_modernirc modern;
    unsigned int retry_wait;
    time_t retry_at;
    int retry_enabled;
};

static struct network_runtime multinet_runtimes[AMBOT_NETWORKS_MAX];

struct line_context {
    struct network_runtime *runtime;
};

static void dispatch_event(const struct ambot_event *event, void *userdata)
{
    struct network_runtime *runtime = (struct network_runtime *)userdata;
    ambot_commands_handle_event(event, &runtime->commands);
    ambot_hooks_dispatch_event(event, &runtime->hooks);
}

static void handle_line(const char *line, void *userdata)
{
    struct line_context *context = (struct line_context *)userdata;
    struct network_runtime *runtime = context->runtime;
    struct ambot_event event;

    printf("[%s] < %s\n", runtime->config->name, line);
    if (ambot_modernirc_handle_line(runtime->sock, runtime->config,
                                    &runtime->modern, line) != 0) {
        printf("AmBot: network %s modern IRC negotiation failed\n",
               runtime->config->name);
        ambot_net_close_socket(runtime->sock);
        runtime->sock = -1;
        return;
    }

    if (strncmp(line, "PING ", 5) == 0) {
        char response[AMBOT_IRC_LINE_MAX + 1];
        int written = snprintf(response, sizeof(response), "PONG %s", line + 5);
        if (written > 0 && written < (int)sizeof(response))
            (void)ambot_irc_send_line(runtime->sock, response);
        return;
    }

    if (ambot_event_from_irc_line(line, &event)) {
        if (ambot_event_queue_push(&runtime->events, &event) == 0)
            ambot_event_dispatch_pending(&runtime->events, dispatch_event, runtime);
    }
}

static int connect_runtime(struct network_runtime *runtime,
                           struct ambot_rexx *rexx,
                           struct ambot_control *control)
{
    struct ambot_network_config *config = runtime->config;
    const char *host = ambot_network_endpoint_host(config);
    unsigned short port = ambot_network_endpoint_port(config);
    unsigned int i;

    if (ambot_network_validate_security(config) != 0) {
        printf("AmBot: invalid security profile for network %s\n", config->name);
        return -1;
    }

    runtime->sock = ambot_net_connect_ipv4(host, port);
    if (runtime->sock < 0) return -1;

    ambot_irc_framer_init(&runtime->framer);
    ambot_event_queue_init(&runtime->events);
    runtime->commands.sock = runtime->sock;
    runtime->commands.bot_nick = config->nick;
    runtime->commands.owner_nick = config->owner;
    ambot_hooks_init(&runtime->hooks, rexx, control);
    runtime->hooks.script_dir = config->hook_dir;
    ambot_modules_init(&runtime->modules);
    (void)ambot_modules_discover(&runtime->modules, config->module_dir);
    ambot_modernirc_init(&runtime->modern);

    if (ambot_modernirc_start(runtime->sock, config, &runtime->modern) != 0 ||
        ambot_irc_send_registration(runtime->sock,
                                    config->nick,
                                    config->user,
                                    config->pass[0] != '\0' ? config->pass : 0) != 0) {
        ambot_net_close_socket(runtime->sock);
        runtime->sock = -1;
        return -1;
    }

    for (i = 0; i < config->channel_count; ++i) {
        char line[AMBOT_IRC_LINE_MAX + 1];
        int written = snprintf(line, sizeof(line), "JOIN %s", config->channels[i]);
        if (written > 0 && written < (int)sizeof(line))
            (void)ambot_irc_send_line(runtime->sock, line);
    }

    printf("AmBot: network %s connected via %s:%u%s CAP=%s SASL=%s TLS=%s\n",
           config->name,
           host,
           (unsigned int)port,
           config->upstream == AMBOT_UPSTREAM_AMBNC ? " (AmBNC)" : "",
           config->cap_enabled ? "on" : "off",
           config->sasl_plain ? "PLAIN" : "off",
           config->tls_upstream ? "upstream" : "off");
    return 0;
}

int ambot_multinet_run(struct ambot_networks *networks,
                       struct ambot_rexx *rexx,
                       struct ambot_control *control)
{
    struct network_runtime *runtimes = multinet_runtimes;
    int sockets[AMBOT_NETWORKS_MAX];
    unsigned int i;
    unsigned int active = 0;
    unsigned int retrying = 0;

    memset(runtimes, 0, sizeof(multinet_runtimes));
    for (i = 0; i < networks->count; ++i) {
        runtimes[i].config = &networks->items[i];
        runtimes[i].sock = -1;
        if (connect_runtime(&runtimes[i], rexx, control) == 0) {
            ++active;
        } else {
            runtimes[i].retry_enabled = 1;
            runtimes[i].retry_wait = 1;
            runtimes[i].retry_at = time(0) + 1;
            ++retrying;
        }
        sockets[i] = runtimes[i].sock;
    }

    if (networks->count > 0 && runtimes[0].sock >= 0)
        ambot_control_set_socket(control, runtimes[0].sock);

    while (!control->quit_requested && !control->disconnect_requested) {
        unsigned long signals = 0;
        unsigned long ready = 0;
        unsigned long wait_mask = ambot_rexx_signal_mask(rexx) | SIGBREAKF_CTRL_C;
        int rc;

        if (active == 0 && retrying == 0) return AMBOT_MULTINET_RECONNECT;

        if (retrying > 0)
            rc = ambot_net_wait_many_timed(sockets, networks->count, wait_mask,
                                           &signals, &ready, 1);
        else
            rc = ambot_net_wait_many(sockets, networks->count, wait_mask,
                                     &signals, &ready);
        if ((signals & ambot_rexx_signal_mask(rexx)) != 0) {
            ambot_rexx_process(rexx, control);
            if (control->reload_requested) return AMBOT_MULTINET_RELOAD;
        }
        if ((signals & SIGBREAKF_CTRL_C) != 0) {
            control->quit_requested = 1;
            break;
        }
        if (rc < 0) return AMBOT_MULTINET_RECONNECT;

        if (retrying > 0) {
            time_t now = time(0);
            for (i = 0; i < networks->count; ++i) {
                if (!runtimes[i].retry_enabled || now < runtimes[i].retry_at) continue;
                if (connect_runtime(&runtimes[i], rexx, control) == 0) {
                    runtimes[i].retry_enabled = 0;
                    sockets[i] = runtimes[i].sock;
                    ++active;
                    --retrying;
                    printf("AmBot: network %s locally reconnected\n",
                           networks->items[i].name);
                    if (i == 0) ambot_control_set_socket(control, runtimes[i].sock);
                } else {
                    if (runtimes[i].retry_wait < 30) {
                        runtimes[i].retry_wait *= 2;
                        if (runtimes[i].retry_wait > 30) runtimes[i].retry_wait = 30;
                    }
                    runtimes[i].retry_at = now + (time_t)runtimes[i].retry_wait;
                    printf("AmBot: network %s reconnect failed; retry in %u seconds\n",
                           networks->items[i].name, runtimes[i].retry_wait);
                }
            }
        }

        for (i = 0; i < networks->count; ++i) {
            if ((ready & (1UL << i)) != 0 && runtimes[i].sock >= 0) {
                char buffer[AMBOT_MULTINET_RECV_BUFFER];
                int received = ambot_net_recv(runtimes[i].sock, buffer, sizeof(buffer));
                if (received <= 0) {
                    ambot_net_close_socket(runtimes[i].sock);
                    runtimes[i].sock = -1;
                    sockets[i] = -1;
                    printf("AmBot: network %s disconnected; attempting local reconnect\n",
                           networks->items[i].name);
                    if (i == 0) ambot_control_set_socket(control, -1);
                    if (active > 0) --active;
                    runtimes[i].retry_enabled = 1;
                    runtimes[i].retry_wait = 1;
                    runtimes[i].retry_at = time(0) + 1;
                    ++retrying;
                    printf("AmBot: network %s scheduled for local reconnect\n",
                           networks->items[i].name);
                } else {
                    struct line_context context;
                    context.runtime = &runtimes[i];
                    ambot_irc_framer_feed(&runtimes[i].framer,
                                          buffer,
                                          (unsigned int)received,
                                          handle_line,
                                          &context);
                    if (runtimes[i].sock < 0) {
                        sockets[i] = -1;
                        if (active > 0) --active;
                        printf("AmBot: network %s disabled after negotiation failure\n",
                               networks->items[i].name);
                        if (i == 0) ambot_control_set_socket(control, -1);
                    }
                }
            }
        }
    }

    for (i = 0; i < networks->count; ++i) {
        if (runtimes[i].sock >= 0) ambot_net_close_socket(runtimes[i].sock);
    }
    ambot_control_set_socket(control, -1);
    return AMBOT_MULTINET_STOP;
}
