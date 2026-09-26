#include <stdio.h>
#include <string.h>

#include "botai.h"
#include "net.h"

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

void ambot_botai_init(struct ambot_botai *botai)
{
    memset(botai, 0, sizeof(*botai));
    botai->port = 8090;
    botai->timeout_seconds = 30;
    copy_value(botai->path, sizeof(botai->path), "");
    copy_value(botai->expert, sizeof(botai->expert), "auto");
}

static int parse_port(const char *text, unsigned short *port)
{
    unsigned long value = 0;
    if (text == 0 || *text == '\0') return -1;
    while (*text != '\0') {
        if (*text < '0' || *text > '9') return -1;
        value = value * 10UL + (unsigned long)(*text - '0');
        if (value > 65535UL) return -1;
        ++text;
    }
    if (value == 0) return -1;
    *port = (unsigned short)value;
    return 0;
}

int ambot_botai_configure(struct ambot_botai *botai,
                          const char *url,
                          const char *expert,
                          unsigned short timeout_seconds)
{
    const char *p;
    const char *slash;
    const char *colon;
    char authority[AMBOT_BOTAI_HOST_MAX + 8];
    unsigned int n;

    ambot_botai_init(botai);
    if (url == 0 || url[0] == '\0') return -1;
    if (strncmp(url, "http://", 7) != 0) return -1;

    p = url + 7;
    slash = strchr(p, '/');
    n = slash != 0 ? (unsigned int)(slash - p) : (unsigned int)strlen(p);
    if (n == 0 || n >= sizeof(authority)) return -1;
    memcpy(authority, p, n);
    authority[n] = '\0';

    colon = strrchr(authority, ':');
    if (colon != 0) {
        char port_text[6];
        unsigned int pn = (unsigned int)strlen(colon + 1);
        if (pn == 0 || pn >= sizeof(port_text)) return -1;
        memcpy(port_text, colon + 1, pn + 1);
        if (parse_port(port_text, &botai->port) != 0) return -1;
        authority[(unsigned int)(colon - authority)] = '\0';
    }
    if (authority[0] == '\0') return -1;

    copy_value(botai->host, sizeof(botai->host), authority);
    if (slash != 0 && slash[1] != '\0') {
        if (strlen(slash) >= sizeof(botai->path)) return -1;
        copy_value(botai->path, sizeof(botai->path), slash);
    }
    copy_value(botai->expert, sizeof(botai->expert),
               expert != 0 && expert[0] != '\0' ? expert : "auto");
    botai->timeout_seconds = timeout_seconds != 0 ? timeout_seconds : 30;
    botai->enabled = 1;
    return 0;
}

/* HTTP transport and strict BotAI v1 response parsing are added separately so
 * configuration remains testable without a live service or extra dependency. */
int ambot_botai_check(struct ambot_botai *botai)
{
    (void)botai;
    return -1;
}

int ambot_botai_chat(struct ambot_botai *botai,
                     const char *message,
                     char *reply,
                     unsigned int reply_size)
{
    (void)botai; (void)message;
    if (reply != 0 && reply_size != 0) reply[0] = '\0';
    return -1;
}
