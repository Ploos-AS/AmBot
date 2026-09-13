#ifndef AMBOT_EVENTS_H
#define AMBOT_EVENTS_H

#define AMBOT_EVENT_QUEUE_CAPACITY 16
#define AMBOT_EVENT_NICK_MAX 31
#define AMBOT_EVENT_TARGET_MAX 63
#define AMBOT_EVENT_TEXT_MAX 255

enum ambot_event_type {
    AMBOT_EVENT_NONE = 0,
    AMBOT_EVENT_PRIVMSG,
    AMBOT_EVENT_NOTICE,
    AMBOT_EVENT_JOIN,
    AMBOT_EVENT_PART,
    AMBOT_EVENT_QUIT,
    AMBOT_EVENT_KICK,
    AMBOT_EVENT_TOPIC,
    AMBOT_EVENT_NICK
};

struct ambot_event {
    enum ambot_event_type type;
    char nick[AMBOT_EVENT_NICK_MAX + 1];
    char target[AMBOT_EVENT_TARGET_MAX + 1];
    char text[AMBOT_EVENT_TEXT_MAX + 1];
};

struct ambot_event_queue {
    struct ambot_event items[AMBOT_EVENT_QUEUE_CAPACITY];
    unsigned char head;
    unsigned char tail;
    unsigned char count;
    unsigned long dropped;
};

typedef void (*ambot_event_handler)(const struct ambot_event *event, void *userdata);

void ambot_event_queue_init(struct ambot_event_queue *queue);
int ambot_event_from_irc_line(const char *line, struct ambot_event *event);
int ambot_event_queue_push(struct ambot_event_queue *queue, const struct ambot_event *event);
int ambot_event_queue_pop(struct ambot_event_queue *queue, struct ambot_event *event);
void ambot_event_dispatch_pending(struct ambot_event_queue *queue,
                                  ambot_event_handler handler,
                                  void *userdata);
const char *ambot_event_type_name(enum ambot_event_type type);

#endif
