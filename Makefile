CC ?= m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/session.h

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@grep -q 'AMBOT_REXX_PORT "AMBOT"' include/ambot.h
	@grep -q 'AMBOT_VERSION "0.2.0-m2"' include/ambot.h
	@grep -q -- '-m68000' Makefile
	@grep -q 'AMBOT_EVENT_QUEUE_CAPACITY 16' include/events.h
	@grep -q 'AMBOT_EVENT_PRIVMSG' include/events.h
	@grep -q 'AMBOT_EVENT_NOTICE' include/events.h
	@grep -q 'AMBOT_EVENT_JOIN' include/events.h
	@grep -q 'AMBOT_EVENT_PART' include/events.h
	@grep -q 'AMBOT_EVENT_QUIT' include/events.h
	@grep -q 'AMBOT_EVENT_KICK' include/events.h
	@grep -q 'AMBOT_EVENT_TOPIC' include/events.h
	@grep -q 'AMBOT_EVENT_NICK' include/events.h
	@grep -q 'ambot_event_dispatch_pending' src/session.c
	@grep -q 'PING ' src/session.c
	@grep -q 'PONG ' src/session.c
	@echo "M2 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
