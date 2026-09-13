#ifndef AMBOT_IRC_H
#define AMBOT_IRC_H

#define AMBOT_IRC_LINE_MAX 512

struct ambot_irc_framer {
    char line[AMBOT_IRC_LINE_MAX + 1];
    unsigned short length;
    unsigned char dropping;
};

typedef void (*ambot_irc_line_cb)(const char *line, void *userdata);

void ambot_irc_framer_init(struct ambot_irc_framer *framer);
void ambot_irc_framer_feed(struct ambot_irc_framer *framer,
                           const char *data,
                           unsigned int length,
                           ambot_irc_line_cb callback,
                           void *userdata);
int ambot_irc_send_line(int sock, const char *line);
int ambot_irc_send_registration(int sock,
                                const char *nick,
                                const char *user,
                                const char *pass);

#endif
