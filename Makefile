ifeq ($(origin CC), default)
CC := m68k-amigaos-gcc
endif
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c src/rexx.c src/hooks.c src/config.c src/modules.c src/networks.c src/modernirc.c src/multinet.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/operations.h include/rexx.h include/hooks.h include/config.h include/modules.h include/networks.h include/modernirc.h include/multinet.h include/session.h

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@test -f src/modernirc.c
	@test -f include/modernirc.h
	@test -f docs/M8.md
	@test -f examples/AmBot-Modern.cfg
	@echo "M8 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
