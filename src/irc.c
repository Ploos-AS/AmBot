#include <stdio.h>
#include <string.h>

#include "irc.h"
#include "net.h"

void ambot_irc_framer_init(struct ambot_irc_framer *framer)
{
    framer->length = 0;
    framer->dropping = 0;
    framer->line[0] = '\0';
}

void ambot_irc_framer_feed(struct ambot_irc_framer *framer,
                           const char *data,
                           unsigned int length,
                           ambot_irc_line_cb callback,
                           void *userdata)
{
    unsigned int i;

    for (i = 0; i < length; ++i) {
        char ch = data[i];

        if (ch == '\n') {
            if (!framer->dropping) {
                if (framer->length > 0 && framer->line[framer->length - 1] == '\r') {
                    --framer->length;
                }
                framer->line[framer->length] = '\0';
                callback(framer->line, userdata);
            }
            framer->length = 0;
            framer->dropping = 0;
            continue;
        }

        if (framer->dropping) {
            continue;
        }

        if (framer->length >= AMBOT_IRC_LINE_MAX) {
            framer->length = 0;
            framer->dropping = 1;
            continue;
        }

        framer->line[framer->length++] = ch;
    }
}

int ambot_irc_send_line(int sock, const char *line)
{
    static const char ending[] = "\r\n";
    unsigned int length = (unsigned int)strlen(line);

    if (ambot_net_send_all(sock, line, length) != 0) {
        return -1;
    }
    return ambot_net_send_all(sock, ending, 2);
}

int ambot_irc_send_registration(int sock,
                                const char *nick,
                                const char *user,
                                const char *pass)
{
    char line[AMBOT_IRC_LINE_MAX + 1];

    if (pass != 0 && pass[0] != '\0') {
        if (snprintf(line, sizeof(line), "PASS %s", pass) < 0 ||
            ambot_irc_send_line(sock, line) != 0) {
            return -1;
        }
    }

    if (snprintf(line, sizeof(line), "NICK %s", nick) < 0 ||
        ambot_irc_send_line(sock, line) != 0) {
        return -1;
    }

    if (snprintf(line, sizeof(line), "USER %s 0 * :AmBot", user) < 0 ||
        ambot_irc_send_line(sock, line) != 0) {
        return -1;
    }

    return 0;
}
