#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"

static void copy_value(char *dst, unsigned int size, const char *src)
{
    unsigned int n;
    if (size == 0) return;
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

void ambot_config_init(struct ambot_config *config)
{
    memset(config, 0, sizeof(*config));
    config->port = 6667;
    copy_value(config->hook_dir, sizeof(config->hook_dir), "REXX:AmBot");
    copy_value(config->module_dir, sizeof(config->module_dir), "REXX:AmBot/Modules");
}

int ambot_config_load(struct ambot_config *config, const char *path)
{
    FILE *file;
    char line[320];

    if (path == 0 || path[0] == '\0') return 0;
    file = fopen(path, "r");
    if (file == 0) return -1;
    copy_value(config->path, sizeof(config->path), path);
    config->channel_count = 0;

    while (fgets(line, sizeof(line), file) != 0) {
        char *key;
        char *value;
        char *equals;
        key = trim(line);
        if (*key == '\0' || *key == '#' || *key == ';') continue;
        equals = strchr(key, '=');
        if (equals == 0) continue;
        *equals = '\0';
        value = trim(equals + 1);
        key = trim(key);

        if (key_equal(key, "HOST")) copy_value(config->host, sizeof(config->host), value);
        else if (key_equal(key, "PORT")) {
            long port = strtol(value, 0, 10);
            if (port > 0 && port <= 65535) config->port = (unsigned short)port;
        }
        else if (key_equal(key, "NICK")) copy_value(config->nick, sizeof(config->nick), value);
        else if (key_equal(key, "USER")) copy_value(config->user, sizeof(config->user), value);
        else if (key_equal(key, "PASS")) copy_value(config->pass, sizeof(config->pass), value);
        else if (key_equal(key, "OWNER")) copy_value(config->owner, sizeof(config->owner), value);
        else if (key_equal(key, "HOOK_DIR")) copy_value(config->hook_dir, sizeof(config->hook_dir), value);
        else if (key_equal(key, "MODULE_DIR")) copy_value(config->module_dir, sizeof(config->module_dir), value);
        else if (key_equal(key, "CHANNEL") && config->channel_count < AMBOT_CONFIG_CHANNELS_MAX) {
            copy_value(config->channels[config->channel_count], sizeof(config->channels[0]), value);
            ++config->channel_count;
        }
    }

    fclose(file);
    return 0;
}
