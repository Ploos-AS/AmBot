#include <stdio.h>
#include <string.h>

#include "ambot.h"
#include "pbmp.h"

static int write_response(char *out, size_t size, const char *text)
{
    int n;
    if (!out || size == 0 || !text) return -1;
    n = snprintf(out, size, "%s", text);
    return n < 0 || (size_t)n >= size ? -1 : 0;
}

static int request_id(const char *request, char *id, size_t size)
{
    const char *p, *end;
    size_t n;
    if (!request || !id || size < 2) return -1;
    p = strstr(request, "\"id\":");
    if (!p) return -1;
    p += 5;
    if (*p != '"') return -1;
    ++p;
    end = strchr(p, '"');
    if (!end) return -1;
    n = (size_t)(end - p);
    if (n == 0 || n >= size) return -1;
    memcpy(id, p, n);
    id[n] = '\0';
    return 0;
}

static int request_has(const char *request, const char *method)
{
    char needle[96];
    if (!request || !method) return 0;
    if (snprintf(needle, sizeof(needle), "\"method\":\"%s\"", method) < 0) return 0;
    return strstr(request, needle) != 0;
}

int ambot_pbmp_response(const struct ambot_pbmp_state *state,
                        const char *request,
                        char *response,
                        size_t response_size)
{
    const char *nick = state && state->nick ? state->nick : "AmBot";
    const char *network_id = state && state->network_id ? state->network_id : "default";
    const char *network_name = state && state->network_name ? state->network_name : network_id;
    const char *connection = state && state->connected ? "connected" : "disconnected";
    char id[96];
    int n;

    if (!request || !strstr(request, "\"pbmp\":1") ||
        !strstr(request, "\"type\":\"request\""))
        return write_response(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"0\",\"ok\":false,\"error\":{\"code\":\"invalid_request\"}}");

    if (request_id(request, id, sizeof(id)) != 0) return -1;

    if (request_has(request, "pbmp.info")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"%s\",\"ok\":true,\"result\":{\"version\":1,\"implementation\":{\"name\":\"ambot\",\"version\":\"" AMBOT_VERSION "\"}}}", id);
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    if (request_has(request, "capabilities.list")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"%s\",\"ok\":true,\"result\":{\"methods\":[\"pbmp.info\",\"capabilities.list\",\"bot.info\",\"networks.list\"]}}", id);
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    if (request_has(request, "bot.info")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"%s\",\"ok\":true,\"result\":{\"bot\":{\"id\":\"ambot\",\"nick\":\"%s\",\"state\":\"%s\",\"implementation\":{\"name\":\"ambot\",\"version\":\"" AMBOT_VERSION "\"}}}}",
            id, nick, state && state->connected ? "running" : "stopped");
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    if (request_has(request, "networks.list")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"%s\",\"ok\":true,\"result\":{\"networks\":[{\"id\":\"%s\",\"name\":\"%s\",\"state\":\"%s\"}]}}",
            id, network_id, network_name, connection);
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    n = snprintf(response, response_size,
        "{\"pbmp\":1,\"type\":\"response\",\"id\":\"%s\",\"ok\":false,\"error\":{\"code\":\"not_supported\"}}", id);
    return n < 0 || (size_t)n >= response_size ? -1 : 0;
}
