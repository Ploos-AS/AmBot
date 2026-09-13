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

#define AMBOT_REXX_SCRIPT_COMMAND_MAX 1024

struct RxsLib *RexxSysBase = 0;
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

static int append_text(char *dst, unsigned int size, unsigned int *length, const char *src)
{
    while (*src != '\0') {
        if (*length + 1 >= size) return -1;
        dst[(*length)++] = *src++;
    }
    dst[*length] = '\0';
    return 0;
}

static int append_quoted_arg(char *dst, unsigned int size, unsigned int *length, const char *src)
{
    if (append_text(dst, size, length, " \"") != 0) return -1;
    while (src != 0 && *src != '\0') {
        char ch = *src++;
        if (ch == '\r' || ch == '\n') ch = ' ';
        if (ch == '"') ch = '\'';
        if (*length + 1 >= size) return -1;
        dst[(*length)++] = ch;
        dst[*length] = '\0';
    }
    return append_text(dst, size, length, "\"");
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
    if (strcmp(command, "RELOAD") == 0)
        return ambot_operation_reload(control, result, size);
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

    snprintf(result, size, "UNKNOWN COMMAND");
    return 10;
}

int ambot_rexx_open(struct ambot_rexx *rexx)
{
    rexx->port = 0;
    RexxSysBase = (struct RxsLib *)OpenLibrary((STRPTR)"rexxsyslib.library", 0);
    if (RexxSysBase == 0) return -1;

    rexx->port = CreateMsgPort();
    if (rexx->port == 0) {
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = 0;
        return -1;
    }

    rexx->port->mp_Node.ln_Name = (char *)ambot_rexx_port_name;
    rexx->port->mp_Node.ln_Pri = 0;

    Forbid();
    if (FindPort((STRPTR)ambot_rexx_port_name) != 0) {
        Permit();
        DeleteMsgPort(rexx->port);
        rexx->port = 0;
        CloseLibrary((struct Library *)RexxSysBase);
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
        CloseLibrary((struct Library *)RexxSysBase);
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
        const char *input = message->rm_Args[0] != 0 ? (const char *)message->rm_Args[0] : "";
        int rc = dispatch_command(control, input, result, sizeof(result));

        message->rm_Result1 = rc;
        message->rm_Result2 = 0;
        if ((message->rm_Action & RXFF_RESULT) != 0) {
            message->rm_Result2 = (LONG)CreateArgstring((UBYTE *)result, (LONG)strlen(result));
        }
        ReplyMsg((struct Message *)message);
    }
}

int ambot_rexx_run_script(struct ambot_rexx *rexx,
                          struct ambot_control *control,
                          const char *script_path,
                          const char *event_name,
                          const char *nick,
                          const char *target,
                          const char *text)
{
    struct MsgPort *master;
    struct MsgPort *reply;
    struct RexxMsg *message;
    char command[AMBOT_REXX_SCRIPT_COMMAND_MAX];
    unsigned int length = 0;
    unsigned long reply_mask;
    unsigned long ambot_mask;
    int done = 0;
    LONG rc;

    if (RexxSysBase == 0 || script_path == 0) return 20;

    master = FindPort((STRPTR)"REXX");
    if (master == 0) return 20;

    reply = CreateMsgPort();
    if (reply == 0) return 20;

    message = CreateRexxMsg(reply, (UBYTE *)".ambot", (UBYTE *)AMBOT_REXX_PORT);
    if (message == 0) {
        DeleteMsgPort(reply);
        return 20;
    }

    command[0] = '\0';
    if (append_text(command, sizeof(command), &length, script_path) != 0 ||
        append_quoted_arg(command, sizeof(command), &length, event_name) != 0 ||
        append_quoted_arg(command, sizeof(command), &length, nick) != 0 ||
        append_quoted_arg(command, sizeof(command), &length, target) != 0 ||
        append_quoted_arg(command, sizeof(command), &length, text) != 0) {
        DeleteRexxMsg(message);
        DeleteMsgPort(reply);
        return 10;
    }

    message->rm_Args[0] = CreateArgstring((UBYTE *)command, (LONG)strlen(command));
    if (message->rm_Args[0] == 0) {
        DeleteRexxMsg(message);
        DeleteMsgPort(reply);
        return 20;
    }
    message->rm_Action = RXCOMM | RXFF_RESULT;

    PutMsg(master, (struct Message *)message);
    reply_mask = 1UL << reply->mp_SigBit;
    ambot_mask = ambot_rexx_signal_mask(rexx);

    while (!done) {
        unsigned long signals = Wait(reply_mask | ambot_mask);
        if ((signals & ambot_mask) != 0) ambot_rexx_process(rexx, control);
        if ((signals & reply_mask) != 0 && GetMsg(reply) != 0) done = 1;
    }

    rc = message->rm_Result1;
    DeleteArgstring(message->rm_Args[0]);
    if (message->rm_Result2 != 0) DeleteArgstring((UBYTE *)message->rm_Result2);
    DeleteRexxMsg(message);
    DeleteMsgPort(reply);
    return (int)rc;
}
