#include <stdio.h>
#include <string.h>

#include "networks.h"

static void copy_value(char *dst, unsigned int size, const char *src)
{
    unsigned int n;
    if (size == 0) return;
    if (src == 0) src = "";
    n = (unsigned int)strlen(src);
    if (n >= size) n = size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

void ambot_networks_init(struct ambot_networks *networks)
{
    memset(networks, 0, sizeof(*networks));
}

int ambot_networks_from_config(struct ambot_networks *networks,
                               const struct ambot_config *config)
{
    struct ambot_network_config *network;
    unsigned int i;

    ambot_networks_init(networks);
    if (config->host[0] == '\0' || config->nick[0] == '\0') return -1;

    network = &networks->items[0];
    copy_value(network->name, sizeof(network->name), "default");
    copy_value(network->host, sizeof(network->host), config->host);
    network->port = config->port;
    copy_value(network->nick, sizeof(network->nick), config->nick);
    copy_value(network->user, sizeof(network->user), config->user);
    copy_value(network->pass, sizeof(network->pass), config->pass);
    copy_value(network->owner, sizeof(network->owner), config->owner);
    copy_value(network->hook_dir, sizeof(network->hook_dir), config->hook_dir);
    copy_value(network->module_dir, sizeof(network->module_dir), config->module_dir);
    for (i = 0; i < config->channel_count; ++i)
        copy_value(network->channels[i], sizeof(network->channels[i]), config->channels[i]);
    network->channel_count = config->channel_count;
    network->upstream = config->use_ambnc ? AMBOT_UPSTREAM_AMBNC : AMBOT_UPSTREAM_DIRECT;
    copy_value(network->ambnc_host, sizeof(network->ambnc_host), config->ambnc_host);
    network->ambnc_port = config->ambnc_port;
    networks->count = 1;
    return 0;
}

const char *ambot_network_endpoint_host(const struct ambot_network_config *network)
{
    if (network->upstream == AMBOT_UPSTREAM_AMBNC && network->ambnc_host[0] != '\0')
        return network->ambnc_host;
    return network->host;
}

unsigned short ambot_network_endpoint_port(const struct ambot_network_config *network)
{
    if (network->upstream == AMBOT_UPSTREAM_AMBNC && network->ambnc_port != 0)
        return network->ambnc_port;
    return network->port;
}
