#include <stdio.h>
#include "ambot.h"

int ambot_run(void)
{
    puts(AMBOT_NAME " " AMBOT_VERSION);
    puts("M0 foundation build");
    puts("ARexx port reserved: " AMBOT_REXX_PORT);
    return 0;
}

int main(void)
{
    return ambot_run();
}
