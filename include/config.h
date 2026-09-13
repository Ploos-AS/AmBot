#ifndef AMBOT_CONFIG_H
#define AMBOT_CONFIG_H

#define AMBOT_CONFIG_PATH_MAX 127
#define AMBOT_CONFIG_HOST_MAX 127
#define AMBOT_CONFIG_NICK_MAX 31
#define AMBOT_CONFIG_USER_MAX 31
#define AMBOT_CONFIG_PASS_MAX 63
#define AMBOT_CONFIG_OWNER_MAX 31
#define AMBOT_CONFIG_CHANNEL_MAX 63
#define AMBOT_CONFIG_CHANNELS_MAX 16

struct ambot_config {
    char path[AMBOT_CONFIG_PATH_MAX + 1];
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
};

void ambot_config_init(struct ambot_config *config);
int ambot_config_load(struct ambot_config *config, const char *path);

#endif
