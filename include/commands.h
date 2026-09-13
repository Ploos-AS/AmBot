#ifndef AMBOT_COMMANDS_H
#define AMBOT_COMMANDS_H

#include "events.h"

enum ambot_permission {
    AMBOT_PERMISSION_USER = 0,
    AMBOT_PERMISSION_ADMIN = 1
};

struct ambot_command_context {
    int sock;
    const char *bot_nick;
    const char *owner_nick;
};

void ambot_commands_handle_event(const struct ambot_event *event, void *userdata);

#endif
