#ifndef AMBOT_MODULES_H
#define AMBOT_MODULES_H

#define AMBOT_MODULES_MAX 16
#define AMBOT_MODULE_NAME_MAX 63
#define AMBOT_MODULE_PATH_MAX 127
#define AMBOT_MODULE_STATE_MAX 63

struct ambot_module {
    char name[AMBOT_MODULE_NAME_MAX + 1];
    char path[AMBOT_MODULE_PATH_MAX + 1];
    char state[AMBOT_MODULE_STATE_MAX + 1];
};

struct ambot_modules {
    struct ambot_module items[AMBOT_MODULES_MAX];
    unsigned char count;
    unsigned long skipped;
};

void ambot_modules_init(struct ambot_modules *modules);
int ambot_modules_discover(struct ambot_modules *modules, const char *directory);
struct ambot_module *ambot_modules_find(struct ambot_modules *modules, const char *name);
int ambot_module_set_state(struct ambot_module *module, const char *state);

#endif
