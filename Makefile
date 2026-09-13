CC ?= m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/session.h

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@grep -q 'AMBOT_REXX_PORT "AMBOT"' include/ambot.h
	@grep -q 'AMBOT_VERSION "0.3.0-m3"' include/ambot.h
	@grep -q -- '-m68000' Makefile
	@grep -q 'AMBOT_COMMAND_PREFIX' src/commands.c
	@grep -q 'HELP' src/commands.c
	@grep -q 'STATUS' src/commands.c
	@grep -q 'VERSION' src/commands.c
	@grep -q 'AMBOT_PERMISSION_ADMIN' include/commands.h
	@grep -q 'owner_nick' include/session.h
	@grep -q 'ambot_commands_handle_event' src/session.c
	@grep -q 'AMBOT_EVENT_QUEUE_CAPACITY 16' include/events.h
	@grep -q 'PING ' src/session.c
	@grep -q 'PONG ' src/session.c
	@echo "M3 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
