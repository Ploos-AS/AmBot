#ifndef AMBOT_MULTINET_H
#define AMBOT_MULTINET_H

struct ambot_networks;
struct ambot_rexx;
struct ambot_control;
struct ambot_botai;

#define AMBOT_MULTINET_STOP 0
#define AMBOT_MULTINET_RELOAD 1
#define AMBOT_MULTINET_RECONNECT 2
#define AMBOT_MULTINET_NO_RETRYABLE_NETWORKS 3

int ambot_multinet_run(struct ambot_networks *networks,
                       struct ambot_rexx *rexx,
                       struct ambot_control *control,
                       struct ambot_botai *botai);

#endif
