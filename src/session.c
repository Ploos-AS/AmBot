#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "commands.h"
#include "events.h"
#include "irc.h"
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

static int ctrl_c_requested(void)
{
    return (SetSignal(0L, 0L) & SIGBREAKF_CTRL_C) != 0;
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
        if (ambot_event_queue_push(context->events, &event) != 0) {
            printf("AmBot: event queue full; dropped=%lu\n", context->events->dropped);
        }
    }
}

static int run_connected_session(const struct ambot_session_config *config,
                                 int sock,
                                 struct ambot_rexx *rexx,
                                 struct ambot_control *control)
{
    struct ambot_irc_framer framer;
    struct ambot_event_queue events;
    struct ambot_command_context commands;
    struct line_context context;
    char buffer[AMBOT_RECV_BUFFER];

    context.sock = sock;
    context.events = &events;
    commands.sock = sock;
    commands.bot_nick = config->nick;
    commands.owner_nick = config->owner_nick;

    ambot_irc_framer_init(&framer);
    ambot_event_queue_init(&events);
    ambot_control_set_socket(control, sock);
    control->disconnect_requested = 0;

    if (ambot_irc_send_registration(sock,
                                    config->nick,
                                    config->user,
                                    config->pass) != 0) {
        ambot_control_set_socket(control, -1);
        return -1;
    }

    while (!ctrl_c_requested() && !control->quit_requested &&
           !control->disconnect_requested) {
        unsigned long signals = 0;
        unsigned long wait_mask = ambot_rexx_signal_mask(rexx) | SIGBREAKF_CTRL_C;
        int readable = ambot_net_wait(sock, wait_mask, &signals);

        if ((signals & ambot_rexx_signal_mask(rexx)) != 0)
            ambot_rexx_process(rexx, control);
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

            ambot_irc_framer_feed(&framer,
                                  buffer,
                                  (unsigned int)received,
                                  handle_line,
                                  &context);
            ambot_event_dispatch_pending(&events, ambot_commands_handle_event, &commands);
        }
    }

    ambot_control_set_socket(control, -1);
    return 0;
}

static void reconnect_delay(unsigned long seconds,
                            struct ambot_rexx *rexx,
                            struct ambot_control *control)
{
    unsigned long ticks = seconds * 50UL;

    while (ticks > 0 && control->connect_requested &&
           !control->quit_requested && !ctrl_c_requested()) {
        ambot_rexx_process(rexx, control);
        if (!control->connect_requested || control->quit_requested) break;
        Delay(ticks > 5 ? 5 : ticks);
        ticks = ticks > 5 ? ticks - 5 : 0;
    }
}

int ambot_session_run(const struct ambot_session_config *config)
{
    struct ambot_rexx rexx;
    struct ambot_control control;
    unsigned long backoff = 1;

    ambot_control_init(&control);

    if (ambot_rexx_open(&rexx) != 0) {
        puts("AmBot: unable to create ARexx port AMBOT");
        return 20;
    }
    puts("AmBot: ARexx port AMBOT ready");

    if (ambot_net_open() != 0) {
        puts("AmBot: unable to open bsdsocket.library");
        ambot_rexx_close(&rexx);
        return 20;
    }

    while (!ctrl_c_requested() && !control.quit_requested) {
        int sock;

        ambot_rexx_process(&rexx, &control);
        if (!control.connect_requested) {
            Delay(5);
            continue;
        }

        printf("AmBot: connecting to %s:%u\n", config->host, (unsigned int)config->port);
        sock = ambot_net_connect_ipv4(config->host, config->port);

        if (sock >= 0) {
            puts("AmBot: connected");
            backoff = 1;
            (void)run_connected_session(config, sock, &rexx, &control);
            ambot_net_close_socket(sock);

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
            reconnect_delay(backoff, &rexx, &control);

            if (backoff < AMBOT_RECONNECT_MAX_SECONDS) {
                backoff *= 2;
                if (backoff > AMBOT_RECONNECT_MAX_SECONDS)
                    backoff = AMBOT_RECONNECT_MAX_SECONDS;
            }
        }
    }

    ambot_control_set_socket(&control, -1);
    ambot_net_close();
    ambot_rexx_close(&rexx);
    puts("AmBot: stopped");
    return 0;
}
