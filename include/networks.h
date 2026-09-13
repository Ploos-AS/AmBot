#ifndef AMBOT_NETWORKS_H
#define AMBOT_NETWORKS_H

#include "config.h"

#define AMBOT_NETWORKS_MAX 4
#define AMBOT_NETWORK_NAME_MAX 31

enum ambot_upstream_type {
    AMBOT_UPSTREAM_DIRECT = 0,
    AMBOT_UPSTREAM_AMBNC
};

struct ambot_network_config {
    char name[AMBOT_NETWORK_NAME_MAX + 1];
    char host[AMBOT_CONFIG_HOST_MAX + 1];
    unsigned short port;
    char nick[AMBOT_CONFIG_NICK_MAX + 1];
    char user[AMBOT_CONFIG_USER_MAX + 1];
    char pass[AMBOT_CONFIG_PASS_MAX + 1];
    char owner[AMBOT_CONFIG_OWNER_MAX + 1];
    char hook_dir[AMBOT_CONFIG_PATH_MAX + 1];
    char module_dir[AMBOT_CONFIG_PATH_MAX + 1];
    char channels[AMBOT_CONFIG_CHANNELS_MAX][AMBOT_CONFIG_CHANNEL_MAX + 1];
    unsigned char channel_count;
    enum ambot_upstream_type upstream;
    char ambnc_host[AMBOT_CONFIG_HOST_MAX + 1];
    unsigned short ambnc_port;
};

struct ambot_networks {
    struct ambot_network_config items[AMBOT_NETWORKS_MAX];
    unsigned char count;
    unsigned long skipped;
};

void ambot_networks_init(struct ambot_networks *networks);
int ambot_networks_from_config(struct ambot_networks *networks,
                               const struct ambot_config *config);
const char *ambot_network_endpoint_host(const struct ambot_network_config *network);
unsigned short ambot_network_endpoint_port(const struct ambot_network_config *network);

#endif
