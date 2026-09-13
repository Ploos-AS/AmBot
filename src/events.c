#include <string.h>
#include "events.h"

static void copy_field(char *dst, unsigned int size, const char *src, unsigned int length)
{
    if (size == 0) return;
    if (length >= size) length = size - 1;
    if (length != 0) memcpy(dst, src, length);
    dst[length] = '\0';
}

static enum ambot_event_type type_from_command(const char *command)
{
    if (strcmp(command, "PRIVMSG") == 0) return AMBOT_EVENT_PRIVMSG;
    if (strcmp(command, "NOTICE") == 0) return AMBOT_EVENT_NOTICE;
    if (strcmp(command, "JOIN") == 0) return AMBOT_EVENT_JOIN;
    if (strcmp(command, "PART") == 0) return AMBOT_EVENT_PART;
    if (strcmp(command, "QUIT") == 0) return AMBOT_EVENT_QUIT;
    if (strcmp(command, "KICK") == 0) return AMBOT_EVENT_KICK;
    if (strcmp(command, "TOPIC") == 0) return AMBOT_EVENT_TOPIC;
    if (strcmp(command, "NICK") == 0) return AMBOT_EVENT_NICK;
    return AMBOT_EVENT_NONE;
}

void ambot_event_queue_init(struct ambot_event_queue *queue)
{
    memset(queue, 0, sizeof(*queue));
}

int ambot_event_from_irc_line(const char *line, struct ambot_event *event)
{
    const char *p = line;
    const char *prefix_start = 0;
    const char *prefix_end = 0;
    const char *command_start;
    const char *command_end;
    const char *param1_start = 0;
    const char *param1_end = 0;
    const char *trailing = 0;
    char command[16];

    memset(event, 0, sizeof(*event));

    if (*p == ':') {
        prefix_start = ++p;
        prefix_end = strchr(p, ' ');
        if (prefix_end == 0) return 0;
        p = prefix_end + 1;
    }

    while (*p == ' ') ++p;
    command_start = p;
    command_end = strchr(p, ' ');
    if (command_end == 0) command_end = p + strlen(p);
    copy_field(command, sizeof(command), command_start,
               (unsigned int)(command_end - command_start));
    event->type = type_from_command(command);
    if (event->type == AMBOT_EVENT_NONE) return 0;

    if (prefix_start != 0) {
        const char *bang = memchr(prefix_start, '!', (size_t)(prefix_end - prefix_start));
        const char *nick_end = bang != 0 ? bang : prefix_end;
        copy_field(event->nick, sizeof(event->nick), prefix_start,
                   (unsigned int)(nick_end - prefix_start));
    }

    p = command_end;
    while (*p == ' ') ++p;
    if (*p != '\0' && *p != ':') {
        param1_start = p;
        param1_end = strchr(p, ' ');
        if (param1_end == 0) param1_end = p + strlen(p);
        copy_field(event->target, sizeof(event->target), param1_start,
                   (unsigned int)(param1_end - param1_start));
        p = param1_end;
    }

    while (*p == ' ') ++p;
    if (*p == ':') trailing = p + 1;
    else if (*p != '\0') trailing = p;

    if (event->type == AMBOT_EVENT_QUIT && trailing != 0) {
        copy_field(event->text, sizeof(event->text), trailing,
                   (unsigned int)strlen(trailing));
    } else if (event->type == AMBOT_EVENT_NICK) {
        const char *newnick = trailing != 0 ? trailing : event->target;
        copy_field(event->target, sizeof(event->target), newnick,
                   (unsigned int)strlen(newnick));
    } else if (trailing != 0) {
        copy_field(event->text, sizeof(event->text), trailing,
                   (unsigned int)strlen(trailing));
    }

    return 1;
}

int ambot_event_queue_push(struct ambot_event_queue *queue, const struct ambot_event *event)
{
    if (queue->count >= AMBOT_EVENT_QUEUE_CAPACITY) {
        ++queue->dropped;
        return -1;
    }
    queue->items[queue->tail] = *event;
    queue->tail = (unsigned char)((queue->tail + 1) % AMBOT_EVENT_QUEUE_CAPACITY);
    ++queue->count;
    return 0;
}

int ambot_event_queue_pop(struct ambot_event_queue *queue, struct ambot_event *event)
{
    if (queue->count == 0) return 0;
    *event = queue->items[queue->head];
    queue->head = (unsigned char)((queue->head + 1) % AMBOT_EVENT_QUEUE_CAPACITY);
    --queue->count;
    return 1;
}

void ambot_event_dispatch_pending(struct ambot_event_queue *queue,
                                  ambot_event_handler handler,
                                  void *userdata)
{
    struct ambot_event event;
    while (ambot_event_queue_pop(queue, &event)) handler(&event, userdata);
}

const char *ambot_event_type_name(enum ambot_event_type type)
{
    switch (type) {
    case AMBOT_EVENT_PRIVMSG: return "PRIVMSG";
    case AMBOT_EVENT_NOTICE: return "NOTICE";
    case AMBOT_EVENT_JOIN: return "JOIN";
    case AMBOT_EVENT_PART: return "PART";
    case AMBOT_EVENT_QUIT: return "QUIT";
    case AMBOT_EVENT_KICK: return "KICK";
    case AMBOT_EVENT_TOPIC: return "TOPIC";
    case AMBOT_EVENT_NICK: return "NICK";
    default: return "NONE";
    }
}
