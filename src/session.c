#include <stdio.h>
#include <string.h>

#include <dos/dos.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "commands.h"
#include "events.h"
#include "irc.h"
#include "net.h"
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

static int run_connected_session(const struct ambot_session_config *config, int sock)
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

    if (ambot_irc_send_registration(sock,
                                    config->nick,
                                    config->user,
                                    config->pass) != 0) {
        return -1;
    }

    while (!ctrl_c_requested()) {
        int received = ambot_net_recv(sock, buffer, sizeof(buffer));
        if (received <= 0) {
            return -1;
        }

        ambot_irc_framer_feed(&framer,
                              buffer,
                              (unsigned int)received,
                              handle_line,
                              &context);
        ambot_event_dispatch_pending(&events, ambot_commands_handle_event, &commands);
    }

    return 0;
}

int ambot_session_run(const struct ambot_session_config *config)
{
    unsigned long backoff = 1;

    if (ambot_net_open() != 0) {
        puts("AmBot: unable to open bsdsocket.library");
        return 20;
    }

    while (!ctrl_c_requested()) {
        int sock;

        printf("AmBot: connecting to %s:%u\n", config->host, (unsigned int)config->port);
        sock = ambot_net_connect_ipv4(config->host, config->port);

        if (sock >= 0) {
            puts("AmBot: connected");
            backoff = 1;
            (void)run_connected_session(config, sock);
            ambot_net_close_socket(sock);

            if (ctrl_c_requested()) {
                break;
            }

            puts("AmBot: connection lost");
        } else {
            puts("AmBot: connection failed");
        }

        printf("AmBot: reconnecting in %lu second(s)\n", backoff);
        Delay(backoff * 50UL);

        if (backoff < AMBOT_RECONNECT_MAX_SECONDS) {
            backoff *= 2;
            if (backoff > AMBOT_RECONNECT_MAX_SECONDS) {
                backoff = AMBOT_RECONNECT_MAX_SECONDS;
            }
        }
    }

    ambot_net_close();
    puts("AmBot: stopped");
    return 0;
}
