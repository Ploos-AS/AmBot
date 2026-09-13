#include <stdio.h>
#include <stdlib.h>

#include "ambot.h"
#include "session.h"

static void usage(const char *program)
{
    printf("Usage: %s HOST PORT NICK [USER] [PASS] [OWNER] [CONFIG]\n", program);
    puts("Example: AmBot irc.libera.chat 6667 AmBot ambot secret MyNick PROGDIR:AmBot.cfg");
}

int ambot_run(int argc, char **argv)
{
    struct ambot_session_config config;
    long port;

    puts(AMBOT_NAME " " AMBOT_VERSION);
    puts("ARexx port reserved: " AMBOT_REXX_PORT);

    if (argc < 4 || argc > 8) {
        usage(argv[0]);
        return 10;
    }

    port = strtol(argv[2], 0, 10);
    if (port <= 0 || port > 65535) {
        puts("AmBot: invalid TCP port");
        return 10;
    }

    config.host = argv[1];
    config.port = (unsigned short)port;
    config.nick = argv[3];
    config.user = argc >= 5 ? argv[4] : argv[3];
    config.pass = argc >= 6 ? argv[5] : 0;
    config.owner_nick = argc >= 7 ? argv[6] : 0;
    config.config_path = argc >= 8 ? argv[7] : 0;

    return ambot_session_run(&config);
}

int main(int argc, char **argv)
{
    return ambot_run(argc, argv);
}
