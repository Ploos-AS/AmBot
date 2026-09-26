#ifndef AMBOT_BOTAI_H
#define AMBOT_BOTAI_H

#define AMBOT_BOTAI_API_VERSION "1.0.0"
#define AMBOT_BOTAI_HOST_MAX 127
#define AMBOT_BOTAI_PATH_MAX 127
#define AMBOT_BOTAI_EXPERT_MAX 31
#define AMBOT_BOTAI_REPLY_MAX 1024
#define AMBOT_BOTAI_RESPONSE_MAX 4096

struct ambot_botai {
    char host[AMBOT_BOTAI_HOST_MAX + 1];
    char path[AMBOT_BOTAI_PATH_MAX + 1];
    char expert[AMBOT_BOTAI_EXPERT_MAX + 1];
    unsigned short port;
    unsigned short timeout_seconds;
    unsigned char enabled;
    unsigned char compatible;
};

void ambot_botai_init(struct ambot_botai *botai);
int ambot_botai_configure(struct ambot_botai *botai,
                          const char *url,
                          const char *expert,
                          unsigned short timeout_seconds);
int ambot_botai_check(struct ambot_botai *botai);
int ambot_botai_chat(struct ambot_botai *botai,
                     const char *message,
                     char *reply,
                     unsigned int reply_size);

#endif
