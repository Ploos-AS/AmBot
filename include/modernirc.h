#ifndef AMBOT_MODERNIRC_H
#define AMBOT_MODERNIRC_H

struct ambot_network_config;

struct ambot_modernirc {
    unsigned char cap_active;
    unsigned char sasl_requested;
    unsigned char sasl_complete;
    unsigned char cap_ended;
};

void ambot_modernirc_init(struct ambot_modernirc *state);
int ambot_modernirc_start(int sock,
                          const struct ambot_network_config *config,
                          struct ambot_modernirc *state);
int ambot_modernirc_handle_line(int sock,
                                const struct ambot_network_config *config,
                                struct ambot_modernirc *state,
                                const char *line);

#endif
