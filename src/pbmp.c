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
    int n;

    if (!request || !strstr(request, "\"pbmp\":1") ||
        !strstr(request, "\"type\":\"request\""))
        return write_response(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"0\",\"ok\":false,\"error\":{\"code\":\"invalid_request\"}}");

    if (request_has(request, "pbmp.info"))
        return write_response(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"1\",\"ok\":true,\"result\":{\"version\":1,\"implementation\":{\"name\":\"ambot\",\"version\":\"" AMBOT_VERSION "\"}}}");

    if (request_has(request, "capabilities.list"))
        return write_response(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"1\",\"ok\":true,\"result\":{\"methods\":[\"pbmp.info\",\"capabilities.list\",\"bot.info\",\"networks.list\"]}}");

    if (request_has(request, "bot.info")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"1\",\"ok\":true,\"result\":{\"bot\":{\"id\":\"ambot\",\"nick\":\"%s\",\"state\":\"%s\",\"implementation\":{\"name\":\"ambot\",\"version\":\"" AMBOT_VERSION "\"}}}}",
            nick, state && state->connected ? "running" : "stopped");
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    if (request_has(request, "networks.list")) {
        n = snprintf(response, response_size,
            "{\"pbmp\":1,\"type\":\"response\",\"id\":\"1\",\"ok\":true,\"result\":{\"networks\":[{\"id\":\"%s\",\"name\":\"%s\",\"state\":\"%s\"}]}}",
            network_id, network_name, connection);
        return n < 0 || (size_t)n >= response_size ? -1 : 0;
    }

    return write_response(response, response_size,
        "{\"pbmp\":1,\"type\":\"response\",\"id\":\"1\",\"ok\":false,\"error\":{\"code\":\"not_supported\"}}");
}
