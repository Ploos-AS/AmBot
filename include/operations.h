#ifndef AMBOT_OPERATIONS_H
#define AMBOT_OPERATIONS_H

#define AMBOT_OPERATION_RESULT_MAX 256

struct ambot_control {
    int sock;
    unsigned char connected;
    unsigned char connect_requested;
    unsigned char disconnect_requested;
    unsigned char quit_requested;
};

void ambot_control_init(struct ambot_control *control);
void ambot_control_set_socket(struct ambot_control *control, int sock);
int ambot_operation_status(const struct ambot_control *control, char *result, unsigned int size);
int ambot_operation_connect(struct ambot_control *control, char *result, unsigned int size);
int ambot_operation_disconnect(struct ambot_control *control, char *result, unsigned int size);
int ambot_operation_send_raw(struct ambot_control *control, const char *line, char *result, unsigned int size);
int ambot_operation_send_target(struct ambot_control *control, const char *command, const char *target, const char *text, char *result, unsigned int size);
int ambot_operation_quit(struct ambot_control *control, char *result, unsigned int size);

#endif
