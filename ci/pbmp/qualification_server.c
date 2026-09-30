#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "pbmp.h"

static volatile sig_atomic_t running = 1;
static void stop_server(int sig) { (void)sig; running = 0; }

int main(void)
{
    const char *path = getenv("AMBOT_PBMP_SOCKET");
    int server;
    struct sockaddr_un addr;
    struct ambot_pbmp_state state = {"AmBot-Q", "qualification", "qualification", 0};

    if (!path || !*path) path = "/tmp/ambot.pbmp.sock";
    if (strlen(path) >= sizeof(addr.sun_path)) return 2;

    signal(SIGINT, stop_server);
    signal(SIGTERM, stop_server);
    unlink(path);

    server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server < 0) { perror("socket"); return 2; }
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path);
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) != 0) { perror("bind"); close(server); return 2; }
    if (listen(server, 4) != 0) { perror("listen"); close(server); unlink(path); return 2; }

    while (running) {
        int client = accept(server, 0, 0);
        char in[4096], out[4096];
        ssize_t n;
        if (client < 0) {
            if (errno == EINTR) continue;
            break;
        }
        n = read(client, in, sizeof(in) - 1);
        if (n > 0) {
            in[n] = '\0';
            if (ambot_pbmp_response(&state, in, out, sizeof(out)) == 0) {
                size_t len = strlen(out);
                out[len++] = '\n';
                (void)write(client, out, len);
            }
        }
        close(client);
    }

    close(server);
    unlink(path);
    return 0;
}
