#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "ambot.h"
#include "commands.h"
#include "irc.h"

#define AMBOT_COMMAND_PREFIX '!'

struct command_def {
    const char *name;
    enum ambot_permission minimum;
    const char *help;
};

static const struct command_def commands[] = {
    { "HELP", AMBOT_PERMISSION_USER, "!HELP - list commands" },
    { "STATUS", AMBOT_PERMISSION_USER, "!STATUS - show bot status" },
    { "VERSION", AMBOT_PERMISSION_USER, "!VERSION - show version" }
};

static int nick_equal(const char *a, const char *b)
{
    if (a == 0 || b == 0) return 0;
    while (*a && *b) {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) return 0;
        ++a; ++b;
    }
    return *a == '\0' && *b == '\0';
}

static enum ambot_permission permission_for(const struct ambot_command_context *context,
                                             const char *nick)
{
    if (context->owner_nick != 0 && context->owner_nick[0] != '\0' &&
        nick_equal(context->owner_nick, nick)) {
        return AMBOT_PERMISSION_ADMIN;
    }
    return AMBOT_PERMISSION_USER;
}

static const char *reply_target(const struct ambot_event *event,
                                const struct ambot_command_context *context)
{
    if (nick_equal(event->target, context->bot_nick)) return event->nick;
    return event->target;
}

static void send_reply(const struct ambot_command_context *context,
                       const char *target,
                       const char *text)
{
    char line[AMBOT_IRC_LINE_MAX + 1];
    int written = snprintf(line, sizeof(line), "PRIVMSG %s :%s", target, text);
    if (written > 0 && written < (int)sizeof(line)) {
        (void)ambot_irc_send_line(context->sock, line);
    }
}

static const struct command_def *find_command(const char *name)
{
    unsigned int i;
    for (i = 0; i < sizeof(commands) / sizeof(commands[0]); ++i) {
        if (nick_equal(commands[i].name, name)) return &commands[i];
    }
    return 0;
}

void ambot_commands_handle_event(const struct ambot_event *event, void *userdata)
{
    struct ambot_command_context *context = (struct ambot_command_context *)userdata;
    char name[16];
    unsigned int i = 0;
    const char *p;
    const char *target;
    const struct command_def *command;
    enum ambot_permission permission;

    if (event->type != AMBOT_EVENT_PRIVMSG || event->text[0] != AMBOT_COMMAND_PREFIX) return;

    p = event->text + 1;
    while (*p && *p != ' ' && i + 1 < sizeof(name)) name[i++] = *p++;
    name[i] = '\0';
    if (name[0] == '\0') return;

    command = find_command(name);
    target = reply_target(event, context);
    if (command == 0) {
        send_reply(context, target, "Unknown command. Try !HELP");
        return;
    }

    permission = permission_for(context, event->nick);
    if (permission < command->minimum) {
        send_reply(context, target, "Permission denied");
        return;
    }

    if (nick_equal(command->name, "HELP")) {
        send_reply(context, target, "Commands: !HELP !STATUS !VERSION");
    } else if (nick_equal(command->name, "STATUS")) {
        send_reply(context, target,
                   permission == AMBOT_PERMISSION_ADMIN ? "AmBot status: online (admin)" : "AmBot status: online");
    } else if (nick_equal(command->name, "VERSION")) {
        send_reply(context, target, AMBOT_NAME " " AMBOT_VERSION);
    }
}
