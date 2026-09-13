CC ?= m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c src/rexx.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/operations.h include/rexx.h include/session.h

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@grep -q 'AMBOT_REXX_PORT "AMBOT"' include/ambot.h
	@grep -q 'AMBOT_VERSION "0.4.0-m4"' include/ambot.h
	@grep -q -- '-m68000' Makefile
	@grep -q 'rexxsyslib.library' src/rexx.c
	@grep -q 'AddPort' src/rexx.c
	@grep -q 'RXFF_RESULT' src/rexx.c
	@grep -q 'STATUS' src/rexx.c
	@grep -q 'CONNECT' src/rexx.c
	@grep -q 'DISCONNECT' src/rexx.c
	@grep -q 'JOIN' src/rexx.c
	@grep -q 'MSG' src/rexx.c
	@grep -q 'NOTICE' src/rexx.c
	@grep -q 'ACTION' src/rexx.c
	@grep -q 'WHOIS' src/rexx.c
	@grep -q 'MODE' src/rexx.c
	@grep -q 'TOPIC' src/rexx.c
	@grep -q 'RAW' src/rexx.c
	@grep -q 'RELOAD' src/rexx.c
	@grep -q 'QUIT' src/rexx.c
	@grep -q 'WaitSelect' src/net.c
	@grep -q 'ambot_rexx_process' src/session.c
	@grep -q 'ambot_operation_send_raw' src/operations.c
	@echo "M4 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
