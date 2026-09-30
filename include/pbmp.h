#ifndef AMBOT_PBMP_H
#define AMBOT_PBMP_H

#include <stddef.h>

struct ambot_pbmp_state {
    const char *nick;
    const char *network_id;
    const char *network_name;
    int connected;
};

int ambot_pbmp_response(const struct ambot_pbmp_state *state,
                        const char *request,
                        char *response,
                        size_t response_size);

#endif
