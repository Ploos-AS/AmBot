#include <stdio.h>
#include <string.h>
#include "pbmp.h"

static int check(const struct ambot_pbmp_state *state, const char *method, const char *needle)
{
    char req[256], out[2048];
    snprintf(req, sizeof(req), "{\"pbmp\":1,\"type\":\"request\",\"id\":\"1\",\"method\":\"%s\",\"params\":{}}", method);
    if (ambot_pbmp_response(state, req, out, sizeof(out)) != 0) return 1;
    if (!strstr(out, "\"ok\":true") || !strstr(out, needle)) {
        fprintf(stderr, "%s failed: %s\n", method, out);
        return 1;
    }
    return 0;
}

int main(void)
{
    struct ambot_pbmp_state state = {"AmBot-Q", "irc-example", "irc.example.invalid", 0};
    if (check(&state, "pbmp.info", "\"name\":\"ambot\"")) return 1;
    if (check(&state, "capabilities.list", "\"networks.list\"")) return 1;
    if (check(&state, "bot.info", "\"id\":\"ambot\"")) return 1;
    if (check(&state, "networks.list", "\"id\":\"irc-example\"")) return 1;
    puts("AmBot PBMP/1 M0 unit checks: PASS");
    return 0;
}
