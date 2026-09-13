#include <stdio.h>
#include <string.h>

#include "irc.h"
#include "operations.h"

static void set_result(char *result, unsigned int size, const char *text)
{
    if (size == 0) return;
    if (text == 0) text = "";
    strncpy(result, text, size - 1);
    result[size - 1] = '\0';
}

void ambot_control_init(struct ambot_control *control)
{
    memset(control, 0, sizeof(*control));
    control->sock = -1;
    control->connect_requested = 1;
}

void ambot_control_set_socket(struct ambot_control *control, int sock)
{
    control->sock = sock;
    control->connected = sock >= 0 ? 1 : 0;
    if (sock >= 0) control->connect_requested = 0;
}

int ambot_operation_status(const struct ambot_control *control, char *result, unsigned int size)
{
    snprintf(result, size, "CONNECTED=%u CONNECT_REQUESTED=%u DISCONNECT_REQUESTED=%u RELOAD_REQUESTED=%u QUIT_REQUESTED=%u",
             (unsigned int)control->connected,
             (unsigned int)control->connect_requested,
             (unsigned int)control->disconnect_requested,
             (unsigned int)control->reload_requested,
             (unsigned int)control->quit_requested);
    return 0;
}

int ambot_operation_connect(struct ambot_control *control, char *result, unsigned int size)
{
    if (control->connected) {
        set_result(result, size, "ALREADY CONNECTED");
        return 0;
    }
    control->connect_requested = 1;
    control->disconnect_requested = 0;
    set_result(result, size, "CONNECT REQUESTED");
    return 0;
}

int ambot_operation_disconnect(struct ambot_control *control, char *result, unsigned int size)
{
    control->disconnect_requested = 1;
    control->connect_requested = 0;
    set_result(result, size, control->connected ? "DISCONNECT REQUESTED" : "DISCONNECTED");
    return 0;
}

int ambot_operation_reload(struct ambot_control *control, char *result, unsigned int size)
{
    control->reload_requested = 1;
    set_result(result, size, "RELOAD REQUESTED");
    return 0;
}

int ambot_operation_send_raw(struct ambot_control *control, const char *line, char *result, unsigned int size)
{
    if (!control->connected || control->sock < 0) {
        set_result(result, size, "NOT CONNECTED");
        return 10;
    }
    if (line == 0 || line[0] == '\0') {
        set_result(result, size, "MISSING LINE");
        return 10;
    }
    if (ambot_irc_send_line(control->sock, line) != 0) {
        set_result(result, size, "SEND FAILED");
        return 20;
    }
    set_result(result, size, "OK");
    return 0;
}

int ambot_operation_send_target(struct ambot_control *control, const char *command, const char *target, const char *text, char *result, unsigned int size)
{
    char line[AMBOT_IRC_LINE_MAX + 1];
    int written;

    if (target == 0 || target[0] == '\0') {
        set_result(result, size, "MISSING TARGET");
        return 10;
    }

    if (text != 0 && text[0] != '\0')
        written = snprintf(line, sizeof(line), "%s %s :%s", command, target, text);
    else
        written = snprintf(line, sizeof(line), "%s %s", command, target);

    if (written <= 0 || written >= (int)sizeof(line)) {
        set_result(result, size, "LINE TOO LONG");
        return 10;
    }
    return ambot_operation_send_raw(control, line, result, size);
}

int ambot_operation_quit(struct ambot_control *control, char *result, unsigned int size)
{
    control->quit_requested = 1;
    control->disconnect_requested = 1;
    control->connect_requested = 0;
    set_result(result, size, "QUIT REQUESTED");
    return 0;
}
