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

static int http_request(struct ambot_botai *botai, const char *method,
                        const char *endpoint, const char *body,
                        char *response, unsigned int response_size)
{
    char request[1536], target[AMBOT_BOTAI_PATH_MAX + 64];
    unsigned int used = 0;
    int sock, written;
    if (botai == 0 || !botai->enabled || response == 0 || response_size < 2) return -1;
    written = snprintf(target, sizeof(target), "%s%s", botai->path, endpoint);
    if (written <= 0 || written >= (int)sizeof(target)) return -1;
    if (body != 0)
        written = snprintf(request, sizeof(request),
            "%s %s HTTP/1.0\r\nHost: %s\r\nContent-Type: application/json\r\nContent-Length: %u\r\nConnection: close\r\n\r\n%s",
            method, target, botai->host, (unsigned int)strlen(body), body);
    else
        written = snprintf(request, sizeof(request),
            "%s %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
            method, target, botai->host);
    if (written <= 0 || written >= (int)sizeof(request)) return -1;
    sock = ambot_net_connect_ipv4(botai->host, botai->port);
    if (sock < 0) return -1;
    if (ambot_net_send_all(sock, request, (unsigned int)written) != 0) {
        ambot_net_close_socket(sock); return -1;
    }
    while (used + 1 < response_size) {
        unsigned long ready = 0, signals = 0;
        int rc = ambot_net_wait_many_timed(&sock, 1, 0, &signals, &ready, botai->timeout_seconds);
        int received;
        (void)signals;
        if (rc <= 0 || (ready & 1UL) == 0) { ambot_net_close_socket(sock); return -1; }
        received = ambot_net_recv(sock, response + used, response_size - used - 1);
        if (received == 0) break;
        if (received < 0) { ambot_net_close_socket(sock); return -1; }
        used += (unsigned int)received;
    }
    ambot_net_close_socket(sock);
    response[used] = '\0';
    if (used + 1 == response_size) return -1;
    if (strncmp(response, "HTTP/1.0 200 ", 13) != 0 &&
        strncmp(response, "HTTP/1.1 200 ", 13) != 0) return -1;
    return 0;
}

static const char *http_body(char *response)
{
    char *p = strstr(response, "\r\n\r\n");
    return p != 0 ? p + 4 : 0;
}

int ambot_botai_check(struct ambot_botai *botai)
{
    char response[AMBOT_BOTAI_RESPONSE_MAX];
    const char *body;
    size_t body_len;
    if (http_request(botai, "GET", "/v1/version", 0, response, sizeof(response)) != 0) {
        if (botai != 0) botai->compatible = 0;
        return -1;
    }
    body = http_body(response);
    if (body == 0) {
        botai->compatible = 0;
        return -1;
    }
    body_len = strlen(body);
    while (body_len > 0 &&
           (body[body_len - 1] == '\n' || body[body_len - 1] == '\r' ||
            body[body_len - 1] == ' ' || body[body_len - 1] == '\t')) {
        --body_len;
    }
    if (body_len != strlen("{\"api_version\":\"1.0.0\"}") ||
        strncmp(body, "{\"api_version\":\"1.0.0\"}", body_len) != 0) {
        botai->compatible = 0;
        return -1;
    }
    botai->compatible = 1;
    return 0;
}

static int json_escape(const char *src, char *dst, unsigned int size)
{
    unsigned int used = 0;
    if (src == 0 || dst == 0 || size == 0) return -1;
    while (*src != '\0') {
        unsigned char ch = (unsigned char)*src++;
        if (ch < 32) return -1;
        if (ch == '"' || ch == '\\') {
            if (used + 2 >= size) return -1;
            dst[used++] = '\\'; dst[used++] = (char)ch;
        } else {
            if (used + 1 >= size) return -1;
            dst[used++] = (char)ch;
        }
    }
    dst[used] = '\0';
    return 0;
}

static void skip_json_ws(const char **p)
{
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n') ++*p;
}

static int parse_json_string(const char **p, char *dst, unsigned int dst_size)
{
    unsigned int used = 0;
    if (**p != '"') return -1;
    ++*p;
    while (**p != '\0' && **p != '"') {
        unsigned char ch = (unsigned char)*(*p)++;
        if (ch < 32) return -1;
        if (ch == '\\') {
            ch = (unsigned char)*(*p)++;
            if (ch == '\0') return -1;
            if (ch == 'n' || ch == 'r' || ch == 't') ch = ' ';
            else if (ch != '"' && ch != '\\' && ch != '/') return -1;
        }
        if (dst != 0) {
            if (used + 1 >= dst_size) return -1;
            dst[used++] = (char)ch;
        }
    }
    if (**p != '"') return -1;
    ++*p;
    if (dst != 0) {
        if (dst_size == 0) return -1;
        dst[used] = '\0';
    }
    return 0;
}

static int parse_text_response(const char *body, char *reply, unsigned int reply_size)
{
    const char *p = body;
    char key[32];
    int have_text = 0;
    if (body == 0 || reply == 0 || reply_size == 0) return -1;
    skip_json_ws(&p);
    if (*p++ != '{') return -1;
    skip_json_ws(&p);
    if (*p == '}') return -1;
    for (;;) {
        if (parse_json_string(&p, key, sizeof(key)) != 0) return -1;
        skip_json_ws(&p);
        if (*p++ != ':') return -1;
        skip_json_ws(&p);
        if (strcmp(key, "text") == 0) {
            if (have_text || parse_json_string(&p, reply, reply_size) != 0 || reply[0] == '\0') return -1;
            have_text = 1;
        } else {
            if (parse_json_string(&p, 0, 0) != 0) return -1;
        }
        skip_json_ws(&p);
        if (*p == '}') { ++p; break; }
        if (*p++ != ',') return -1;
        skip_json_ws(&p);
    }
    skip_json_ws(&p);
    return have_text && *p == '\0' ? 0 : -1;
}

int ambot_botai_chat(struct ambot_botai *botai, const char *message,
                     char *reply, unsigned int reply_size)
{
    char escaped[768], body[1024], response[AMBOT_BOTAI_RESPONSE_MAX];
    const char *response_body;
    int written;
    if (reply != 0 && reply_size != 0) reply[0] = '\0';
    if (botai == 0 || !botai->compatible || message == 0 || reply == 0 || reply_size == 0) return -1;
    if (json_escape(message, escaped, sizeof(escaped)) != 0) return -1;
    written = snprintf(body, sizeof(body), "{\"expert\":\"%s\",\"message\":\"%s\"}", botai->expert, escaped);
    if (written <= 0 || written >= (int)sizeof(body)) return -1;
    if (http_request(botai, "POST", "/v1/chat", body, response, sizeof(response)) != 0) return -1;
    response_body = http_body(response);
    return parse_text_response(response_body, reply, reply_size);
}
