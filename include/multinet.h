#ifndef AMBOT_MULTINET_H
#define AMBOT_MULTINET_H

struct ambot_networks;
struct ambot_rexx;
struct ambot_control;

#define AMBOT_MULTINET_STOP 0
#define AMBOT_MULTINET_RELOAD 1
#define AMBOT_MULTINET_RECONNECT 2

int ambot_multinet_run(struct ambot_networks *networks,
                       struct ambot_rexx *rexx,
                       struct ambot_control *control);

#endif
