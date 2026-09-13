#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <exec/libraries.h>
#include <exec/ports.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>

#include "ambot.h"
#include "irc.h"
#include "operations.h"
#include "rexx.h"

struct Library *RexxSysBase = 0;
static char ambot_rexx_port_name[] = AMBOT_REXX_PORT;

static const char *skip_space(const char *p)
{
    while (p != 0 && *p != '\0' && isspace((unsigned char)*p)) ++p;
    return p;
}

static const char *next_word(const char *p, char *word, unsigned int size)
{
    unsigned int n = 0;
    p = skip_space(p);
    if (p == 0) return 0;
    while (*p != '\0' && !isspace((unsigned char)*p)) {
        if (n + 1 < size) word[n++] = *p;
        ++p;
    }
    if (size != 0) word[n] = '\0';
    return skip_space(p);
}

static void uppercase(char *text)
{
    while (*text != '\0') {
        *text = (char)toupper((unsigned char)*text);
        ++text;
    }
}

static int send_command(struct ambot_control *control,
                        const char *command,
                        const char *args,
                        char *result,
                        unsigned int size)
{
    char line[AMBOT_IRC_LINE_MAX + 1];
    int written;

    args = skip_space(args);
    if (args == 0 || args[0] == '\0') {
        snprintf(result, size, "MISSING ARGUMENTS");
        return 10;
    }
    written = snprintf(line, sizeof(line), "%s %s", command, args);
    if (written <= 0 || written >= (int)sizeof(line)) {
        snprintf(result, size, "LINE TOO LONG");
        return 10;
    }
    return ambot_operation_send_raw(control, line, result, size);
}

static int dispatch_command(struct ambot_control *control,
                            const char *input,
                            char *result,
                            unsigned int size)
{
    char command[16];
    char target[64];
    const char *args;
    const char *text;

    args = next_word(input, command, sizeof(command));
    uppercase(command);

    if (command[0] == '\0') {
        snprintf(result, size, "EMPTY COMMAND");
        return 10;
    }
    if (strcmp(command, "STATUS") == 0)
        return ambot_operation_status(control, result, size);
    if (strcmp(command, "CONNECT") == 0)
        return ambot_operation_connect(control, result, size);
    if (strcmp(command, "DISCONNECT") == 0)
        return ambot_operation_disconnect(control, result, size);
    if (strcmp(command, "QUIT") == 0)
        return ambot_operation_quit(control, result, size);
    if (strcmp(command, "RAW") == 0)
        return ambot_operation_send_raw(control, skip_space(args), result, size);
    if (strcmp(command, "JOIN") == 0)
        return send_command(control, "JOIN", args, result, size);
    if (strcmp(command, "WHOIS") == 0)
        return send_command(control, "WHOIS", args, result, size);
    if (strcmp(command, "MODE") == 0)
        return send_command(control, "MODE", args, result, size);
    if (strcmp(command, "PART") == 0 || strcmp(command, "TOPIC") == 0 ||
        strcmp(command, "MSG") == 0 || strcmp(command, "NOTICE") == 0 ||
        strcmp(command, "ACTION") == 0) {
        char payload[AMBOT_IRC_LINE_MAX + 1];
        int written;

        text = next_word(args, target, sizeof(target));
        if (target[0] == '\0') {
            snprintf(result, size, "MISSING TARGET");
            return 10;
        }
        text = skip_space(text);

        if (strcmp(command, "MSG") == 0)
            return ambot_operation_send_target(control, "PRIVMSG", target, text, result, size);
        if (strcmp(command, "NOTICE") == 0)
            return ambot_operation_send_target(control, "NOTICE", target, text, result, size);
        if (strcmp(command, "PART") == 0)
            return ambot_operation_send_target(control, "PART", target, text, result, size);
        if (strcmp(command, "TOPIC") == 0)
            return ambot_operation_send_target(control, "TOPIC", target, text, result, size);

        written = snprintf(payload, sizeof(payload), "PRIVMSG %s :\001ACTION %s\001", target,
                           text != 0 ? text : "");
        if (written <= 0 || written >= (int)sizeof(payload)) {
            snprintf(result, size, "LINE TOO LONG");
            return 10;
        }
        return ambot_operation_send_raw(control, payload, result, size);
    }
    if (strcmp(command, "RELOAD") == 0) {
        snprintf(result, size, "RELOAD DEFERRED UNTIL CONFIG MILESTONE");
        return 5;
    }

    snprintf(result, size, "UNKNOWN COMMAND");
    return 10;
}

int ambot_rexx_open(struct ambot_rexx *rexx)
{
    rexx->port = 0;
    RexxSysBase = OpenLibrary("rexxsyslib.library", 0);
    if (RexxSysBase == 0) return -1;

    rexx->port = CreateMsgPort();
    if (rexx->port == 0) {
        CloseLibrary(RexxSysBase);
        RexxSysBase = 0;
        return -1;
    }

    rexx->port->mp_Node.ln_Name = ambot_rexx_port_name;
    rexx->port->mp_Node.ln_Pri = 0;

    Forbid();
    if (FindPort(ambot_rexx_port_name) != 0) {
        Permit();
        DeleteMsgPort(rexx->port);
        rexx->port = 0;
        CloseLibrary(RexxSysBase);
        RexxSysBase = 0;
        return -1;
    }
    AddPort(rexx->port);
    Permit();
    return 0;
}

void ambot_rexx_close(struct ambot_rexx *rexx)
{
    if (rexx->port != 0) {
        struct Message *message;
        Forbid();
        RemPort(rexx->port);
        Permit();
        while ((message = GetMsg(rexx->port)) != 0) ReplyMsg(message);
        DeleteMsgPort(rexx->port);
        rexx->port = 0;
    }
    if (RexxSysBase != 0) {
        CloseLibrary(RexxSysBase);
        RexxSysBase = 0;
    }
}

unsigned long ambot_rexx_signal_mask(const struct ambot_rexx *rexx)
{
    if (rexx->port == 0) return 0;
    return 1UL << rexx->port->mp_SigBit;
}

void ambot_rexx_process(struct ambot_rexx *rexx, struct ambot_control *control)
{
    struct RexxMsg *message;

    while (rexx->port != 0 && (message = (struct RexxMsg *)GetMsg(rexx->port)) != 0) {
        char result[AMBOT_OPERATION_RESULT_MAX];
        const char *input = message->rm_Args[0] != 0 ? message->rm_Args[0] : "";
        int rc = dispatch_command(control, input, result, sizeof(result));

        message->rm_Result1 = rc;
        message->rm_Result2 = 0;
        if ((message->rm_Action & RXFF_RESULT) != 0) {
            message->rm_Result2 = (LONG)CreateArgstring(result, (LONG)strlen(result));
        }
        ReplyMsg((struct Message *)message);
    }
}
