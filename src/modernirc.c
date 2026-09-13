#include <stdio.h>
#include <string.h>

#include "irc.h"
#include "modernirc.h"
#include "networks.h"

static int send_line(int sock, const char *line)
{
    return ambot_irc_send_line(sock, line);
}

static int contains_token(const char *line, const char *token)
{
    return line != 0 && token != 0 && strstr(line, token) != 0;
}

static int b64_encode(const unsigned char *src, unsigned int length,
                      char *dst, unsigned int size)
{
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int i = 0;
    unsigned int o = 0;

    while (i < length) {
        unsigned long v = (unsigned long)src[i++] << 16;
        unsigned int remain = length - (i - 1);
        if (remain > 1) v |= (unsigned long)src[i++] << 8;
        if (remain > 2) v |= (unsigned long)src[i++];
        if (o + 4 >= size) return -1;
        dst[o++] = table[(v >> 18) & 63];
        dst[o++] = table[(v >> 12) & 63];
        dst[o++] = remain > 1 ? table[(v >> 6) & 63] : '=';
        dst[o++] = remain > 2 ? table[v & 63] : '=';
    }
    if (o >= size) return -1;
    dst[o] = '\0';
    return 0;
}

static int send_sasl_payload(int sock, const struct ambot_network_config *config)
{
    unsigned char plain[160];
    char encoded[224];
    unsigned int ulen = (unsigned int)strlen(config->sasl_user);
    unsigned int plen = (unsigned int)strlen(config->sasl_pass);
    unsigned int length;
    char line[240];

    if (ulen * 2 + plen + 2 > sizeof(plain)) return -1;
    memcpy(plain, config->sasl_user, ulen);
    plain[ulen] = 0;
    memcpy(plain + ulen + 1, config->sasl_user, ulen);
    plain[ulen + 1 + ulen] = 0;
    memcpy(plain + ulen + 2 + ulen, config->sasl_pass, plen);
    length = ulen + 1 + ulen + 1 + plen;

    if (b64_encode(plain, length, encoded, sizeof(encoded)) != 0) return -1;
    if (snprintf(line, sizeof(line), "AUTHENTICATE %s", encoded) >= (int)sizeof(line)) return -1;
    return send_line(sock, line);
}

void ambot_modernirc_init(struct ambot_modernirc *state)
{
    memset(state, 0, sizeof(*state));
}

int ambot_modernirc_start(int sock,
                          const struct ambot_network_config *config,
                          struct ambot_modernirc *state)
{
    ambot_modernirc_init(state);
    if (!config->cap_enabled) return 0;
    state->cap_active = 1;
    return send_line(sock, "CAP LS 302");
}

int ambot_modernirc_handle_line(int sock,
                                const struct ambot_network_config *config,
                                struct ambot_modernirc *state,
                                const char *line)
{
    if (!state->cap_active || state->cap_ended) return 0;

    if (contains_token(line, " CAP ") && contains_token(line, " LS ")) {
        if (config->sasl_plain && contains_token(line, "sasl")) {
            state->sasl_requested = 1;
            return send_line(sock, "CAP REQ :sasl");
        }
        state->cap_ended = 1;
        return send_line(sock, "CAP END");
    }

    if (state->sasl_requested && contains_token(line, " CAP ") &&
        contains_token(line, " ACK ") && contains_token(line, "sasl"))
        return send_line(sock, "AUTHENTICATE PLAIN");

    if (state->sasl_requested && strncmp(line, "AUTHENTICATE +", 14) == 0)
        return send_sasl_payload(sock, config);

    if (contains_token(line, " 903 ")) {
        state->sasl_complete = 1;
        state->cap_ended = 1;
        return send_line(sock, "CAP END");
    }

    if (contains_token(line, " 904 ") || contains_token(line, " 905 ") ||
        contains_token(line, " 906 ") || contains_token(line, " 907 ")) {
        state->cap_ended = 1;
        (void)send_line(sock, "CAP END");
        return -1;
    }

    return 0;
}
