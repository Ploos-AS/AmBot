#ifndef AMBOT_SESSION_H
#define AMBOT_SESSION_H

struct ambot_session_config {
    const char *host;
    unsigned short port;
    const char *nick;
    const char *user;
    const char *pass;
    const char *owner_nick;
    const char *config_path;
};

int ambot_session_run(const struct ambot_session_config *config);

#endif
