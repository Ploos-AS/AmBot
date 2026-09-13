#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"

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

static char *trim(char *text)
{
    char *end;
    while (*text != '\0' && isspace((unsigned char)*text)) ++text;
    end = text + strlen(text);
    while (end > text && isspace((unsigned char)end[-1])) --end;
    *end = '\0';
    return text;
}

static int key_equal(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (toupper((unsigned char)*a) != toupper((unsigned char)*b)) return 0;
        ++a; ++b;
    }
    return *a == '\0' && *b == '\0';
}

static int enabled(const char *value)
{
    return key_equal(value, "ON") || key_equal(value, "YES") ||
           key_equal(value, "TRUE") || strcmp(value, "1") == 0;
}

static unsigned short parse_port(const char *value, unsigned short fallback)
{
    long parsed = strtol(value, 0, 10);
    return parsed > 0 && parsed <= 65535 ? (unsigned short)parsed : fallback;
}

static void network_defaults(struct ambot_config_network *network,
                             const struct ambot_config *config)
{
    memset(network, 0, sizeof(*network));
    network->port = config->port;
    network->ambnc_port = config->ambnc_port;
    copy_value(network->nick, sizeof(network->nick), config->nick);
    copy_value(network->user, sizeof(network->user), config->user);
    copy_value(network->pass, sizeof(network->pass), config->pass);
    copy_value(network->owner, sizeof(network->owner), config->owner);
    copy_value(network->hook_dir, sizeof(network->hook_dir), config->hook_dir);
    copy_value(network->module_dir, sizeof(network->module_dir), config->module_dir);
    copy_value(network->ambnc_host, sizeof(network->ambnc_host), config->ambnc_host);
    network->use_ambnc = config->use_ambnc;
    network->cap_enabled = config->cap_enabled;
    network->sasl_plain = config->sasl_plain;
    copy_value(network->sasl_user, sizeof(network->sasl_user), config->sasl_user);
    copy_value(network->sasl_pass, sizeof(network->sasl_pass), config->sasl_pass);
    network->tls_upstream = config->tls_upstream;
}

void ambot_config_init(struct ambot_config *config)
{
    memset(config, 0, sizeof(*config));
    config->port = 6667;
    config->ambnc_port = 6667;
    config->cap_enabled = 1;
    copy_value(config->hook_dir, sizeof(config->hook_dir), "REXX:AmBot");
    copy_value(config->module_dir, sizeof(config->module_dir), "REXX:AmBot/Modules");
}

void ambot_config_seed(struct ambot_config *config,
                       const char *host,
                       unsigned short port,
                       const char *nick,
                       const char *user,
                       const char *pass,
                       const char *owner)
{
    ambot_config_init(config);
    copy_value(config->host, sizeof(config->host), host);
    config->port = port != 0 ? port : 6667;
    copy_value(config->nick, sizeof(config->nick), nick);
    copy_value(config->user, sizeof(config->user), user != 0 && user[0] != '\0' ? user : nick);
    copy_value(config->pass, sizeof(config->pass), pass);
    copy_value(config->owner, sizeof(config->owner), owner);
}

static void apply_network_key(struct ambot_config_network *network,
                              const char *key,
                              const char *value)
{
    if (key_equal(key, "HOST")) copy_value(network->host, sizeof(network->host), value);
    else if (key_equal(key, "PORT")) network->port = parse_port(value, network->port);
    else if (key_equal(key, "NICK")) copy_value(network->nick, sizeof(network->nick), value);
    else if (key_equal(key, "USER")) copy_value(network->user, sizeof(network->user), value);
    else if (key_equal(key, "PASS")) copy_value(network->pass, sizeof(network->pass), value);
    else if (key_equal(key, "OWNER")) copy_value(network->owner, sizeof(network->owner), value);
    else if (key_equal(key, "HOOK_DIR")) copy_value(network->hook_dir, sizeof(network->hook_dir), value);
    else if (key_equal(key, "MODULE_DIR")) copy_value(network->module_dir, sizeof(network->module_dir), value);
    else if (key_equal(key, "UPSTREAM")) network->use_ambnc = key_equal(value, "AMBNC") ? 1 : 0;
    else if (key_equal(key, "AMBNC_HOST")) copy_value(network->ambnc_host, sizeof(network->ambnc_host), value);
    else if (key_equal(key, "AMBNC_PORT")) network->ambnc_port = parse_port(value, network->ambnc_port);
    else if (key_equal(key, "CAP")) network->cap_enabled = enabled(value) ? 1 : 0;
    else if (key_equal(key, "SASL")) network->sasl_plain = key_equal(value, "PLAIN") ? 1 : 0;
    else if (key_equal(key, "SASL_USER")) copy_value(network->sasl_user, sizeof(network->sasl_user), value);
    else if (key_equal(key, "SASL_PASS")) copy_value(network->sasl_pass, sizeof(network->sasl_pass), value);
    else if (key_equal(key, "TLS")) network->tls_upstream = key_equal(value, "UPSTREAM") ? 1 : 0;
    else if (key_equal(key, "CHANNEL") && network->channel_count < AMBOT_CONFIG_CHANNELS_MAX) {
        copy_value(network->channels[network->channel_count], sizeof(network->channels[0]), value);
        ++network->channel_count;
    }
}

int ambot_config_load(struct ambot_config *config, const char *path)
{
    FILE *file;
    char line[320];
    struct ambot_config_network *current = 0;

    if (path == 0 || path[0] == '\0') return 0;
    file = fopen(path, "r");
    if (file == 0) return -1;
    copy_value(config->path, sizeof(config->path), path);
    config->channel_count = 0;
    config->network_count = 0;
    config->networks_skipped = 0;

    while (fgets(line, sizeof(line), file) != 0) {
        char *key = trim(line);
        char *value;
        char *equals;

        if (*key == '\0' || *key == '#' || *key == ';') continue;
        if (*key == '[') {
            char *end = strchr(key, ']');
            if (end != 0) {
                char *name;
                *end = '\0';
                name = trim(key + 1);
                if (strlen(name) > 8 && strncmp(name, "NETWORK ", 8) == 0) {
                    if (config->network_count < AMBOT_CONFIG_NETWORKS_MAX) {
                        current = &config->networks[config->network_count++];
                        network_defaults(current, config);
                        copy_value(current->name, sizeof(current->name), trim(name + 8));
                    } else {
                        ++config->networks_skipped;
                        current = 0;
                    }
                }
            }
            continue;
        }

        equals = strchr(key, '=');
        if (equals == 0) continue;
        *equals = '\0';
        value = trim(equals + 1);
        key = trim(key);

        if (current != 0) {
            apply_network_key(current, key, value);
            continue;
        }

        if (key_equal(key, "HOST")) copy_value(config->host, sizeof(config->host), value);
        else if (key_equal(key, "PORT")) config->port = parse_port(value, config->port);
        else if (key_equal(key, "NICK")) copy_value(config->nick, sizeof(config->nick), value);
        else if (key_equal(key, "USER")) copy_value(config->user, sizeof(config->user), value);
        else if (key_equal(key, "PASS")) copy_value(config->pass, sizeof(config->pass), value);
        else if (key_equal(key, "OWNER")) copy_value(config->owner, sizeof(config->owner), value);
        else if (key_equal(key, "HOOK_DIR")) copy_value(config->hook_dir, sizeof(config->hook_dir), value);
        else if (key_equal(key, "MODULE_DIR")) copy_value(config->module_dir, sizeof(config->module_dir), value);
        else if (key_equal(key, "UPSTREAM")) config->use_ambnc = key_equal(value, "AMBNC") ? 1 : 0;
        else if (key_equal(key, "AMBNC_HOST")) copy_value(config->ambnc_host, sizeof(config->ambnc_host), value);
        else if (key_equal(key, "AMBNC_PORT")) config->ambnc_port = parse_port(value, config->ambnc_port);
        else if (key_equal(key, "CAP")) config->cap_enabled = enabled(value) ? 1 : 0;
        else if (key_equal(key, "SASL")) config->sasl_plain = key_equal(value, "PLAIN") ? 1 : 0;
        else if (key_equal(key, "SASL_USER")) copy_value(config->sasl_user, sizeof(config->sasl_user), value);
        else if (key_equal(key, "SASL_PASS")) copy_value(config->sasl_pass, sizeof(config->sasl_pass), value);
        else if (key_equal(key, "TLS")) config->tls_upstream = key_equal(value, "UPSTREAM") ? 1 : 0;
        else if (key_equal(key, "CHANNEL") && config->channel_count < AMBOT_CONFIG_CHANNELS_MAX) {
            copy_value(config->channels[config->channel_count], sizeof(config->channels[0]), value);
            ++config->channel_count;
        }
    }

    fclose(file);
    return 0;
}
