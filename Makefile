CC ?= m68k-amigaos-gcc
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean check

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(LDFLAGS)

src/%.o: src/%.c include/ambot.h
	$(CC) $(CFLAGS) -c $< -o $@

check:
	@grep -q 'AMBOT_REXX_PORT "AMBOT"' include/ambot.h
	@grep -q -- '-m68000' Makefile
	@grep -q 'AmigaOS 2.04+' README.md
	@grep -q 'ARexx' docs/ARCHITECTURE.md
	@echo "M0 static checks: PASS"

clean:
	rm -f $(OBJECTS) $(TARGET)
