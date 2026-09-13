#ifndef AMBOT_REXX_H
#define AMBOT_REXX_H

struct MsgPort;
struct ambot_control;

struct ambot_rexx {
    struct MsgPort *port;
};

int ambot_rexx_open(struct ambot_rexx *rexx);
void ambot_rexx_close(struct ambot_rexx *rexx);
unsigned long ambot_rexx_signal_mask(const struct ambot_rexx *rexx);
void ambot_rexx_process(struct ambot_rexx *rexx, struct ambot_control *control);

#endif
