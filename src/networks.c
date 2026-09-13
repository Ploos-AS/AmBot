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

static void copy_network(struct ambot_network_config *dst,
                         const struct ambot_config_network *src)
{
    unsigned int i;
    memset(dst, 0, sizeof(*dst));
    copy_value(dst->name, sizeof(dst->name), src->name);
    copy_value(dst->host, sizeof(dst->host), src->host);
    dst->port = src->port;
    copy_value(dst->nick, sizeof(dst->nick), src->nick);
    copy_value(dst->user, sizeof(dst->user), src->user);
    copy_value(dst->pass, sizeof(dst->pass), src->pass);
    copy_value(dst->owner, sizeof(dst->owner), src->owner);
    copy_value(dst->hook_dir, sizeof(dst->hook_dir), src->hook_dir);
    copy_value(dst->module_dir, sizeof(dst->module_dir), src->module_dir);
    for (i = 0; i < src->channel_count; ++i)
        copy_value(dst->channels[i], sizeof(dst->channels[i]), src->channels[i]);
    dst->channel_count = src->channel_count;
    dst->upstream = src->use_ambnc ? AMBOT_UPSTREAM_AMBNC : AMBOT_UPSTREAM_DIRECT;
    copy_value(dst->ambnc_host, sizeof(dst->ambnc_host), src->ambnc_host);
    dst->ambnc_port = src->ambnc_port;
    dst->cap_enabled = src->cap_enabled;
    dst->sasl_plain = src->sasl_plain;
    copy_value(dst->sasl_user, sizeof(dst->sasl_user), src->sasl_user);
    copy_value(dst->sasl_pass, sizeof(dst->sasl_pass), src->sasl_pass);
    dst->tls_upstream = src->tls_upstream;
}

void ambot_networks_init(struct ambot_networks *networks)
{
    memset(networks, 0, sizeof(*networks));
}

int ambot_networks_from_config(struct ambot_networks *networks,
                               const struct ambot_config *config)
{
    unsigned int i;
    ambot_networks_init(networks);

    if (config->network_count != 0) {
        for (i = 0; i < config->network_count && i < AMBOT_NETWORKS_MAX; ++i)
            copy_network(&networks->items[i], &config->networks[i]);
        networks->count = config->network_count > AMBOT_NETWORKS_MAX ? AMBOT_NETWORKS_MAX : config->network_count;
        networks->skipped = config->networks_skipped;
        for (i = 0; i < networks->count; ++i)
            if (ambot_network_validate_security(&networks->items[i]) != 0) return -1;
        return networks->count != 0 ? 0 : -1;
    }

    if (config->host[0] == '\0' || config->nick[0] == '\0') return -1;
    {
        struct ambot_config_network legacy;
        memset(&legacy, 0, sizeof(legacy));
        copy_value(legacy.name, sizeof(legacy.name), "default");
        copy_value(legacy.host, sizeof(legacy.host), config->host);
        legacy.port = config->port;
        copy_value(legacy.nick, sizeof(legacy.nick), config->nick);
        copy_value(legacy.user, sizeof(legacy.user), config->user);
        copy_value(legacy.pass, sizeof(legacy.pass), config->pass);
        copy_value(legacy.owner, sizeof(legacy.owner), config->owner);
        copy_value(legacy.hook_dir, sizeof(legacy.hook_dir), config->hook_dir);
        copy_value(legacy.module_dir, sizeof(legacy.module_dir), config->module_dir);
        for (i = 0; i < config->channel_count; ++i)
            copy_value(legacy.channels[i], sizeof(legacy.channels[i]), config->channels[i]);
        legacy.channel_count = config->channel_count;
        legacy.use_ambnc = config->use_ambnc;
        copy_value(legacy.ambnc_host, sizeof(legacy.ambnc_host), config->ambnc_host);
        legacy.ambnc_port = config->ambnc_port;
        legacy.cap_enabled = config->cap_enabled;
        legacy.sasl_plain = config->sasl_plain;
        copy_value(legacy.sasl_user, sizeof(legacy.sasl_user), config->sasl_user);
        copy_value(legacy.sasl_pass, sizeof(legacy.sasl_pass), config->sasl_pass);
        legacy.tls_upstream = config->tls_upstream;
        copy_network(&networks->items[0], &legacy);
        networks->count = 1;
    }
    return ambot_network_validate_security(&networks->items[0]);
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

int ambot_network_validate_security(const struct ambot_network_config *network)
{
    if (network->tls_upstream && network->upstream != AMBOT_UPSTREAM_AMBNC) return -1;
    if (network->sasl_plain && (network->sasl_user[0] == '\0' || network->sasl_pass[0] == '\0')) return -1;
    return 0;
}
