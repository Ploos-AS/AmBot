CC ?= m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c src/rexx.c src/hooks.c src/config.c src/modules.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/operations.h include/rexx.h include/hooks.h include/config.h include/modules.h include/session.h

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@test -f src/config.c
	@test -f src/modules.c
	@test -f docs/M6.md
	@test -f examples/AmBot.cfg
	@echo "M6 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
