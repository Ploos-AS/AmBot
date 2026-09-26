ifeq ($(origin CC), default)
CC := m68k-amigaos-gcc
endif
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c src/rexx.c src/hooks.c src/config.c src/modules.c src/networks.c src/modernirc.c src/multinet.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/operations.h include/rexx.h include/hooks.h include/config.h include/modules.h include/networks.h include/modernirc.h include/multinet.h include/session.h

.PHONY: all clean check amiga qualify-m9_2 check-m9_2 review-m9_2-evidence run-m9_2-host

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


check-m9_2:
	@python3 -m py_compile ci/m9_2/fixture_server.py ci/m9_2/tls_proxy.py
	@test -f ci/m9_2/AmBot-M9_2.cfg.in\n\t@test -f ci/m9_2/AmBot-SASL-Failure-M9_2.cfg.in
	@test -f ci/m9_2/rexx/Commands-M9_2.rexx
	@test -f ci/m9_2/rexx/ON_PRIVMSG.rexx\n\t@test -f ci/m9_2/rexx/ON_NOTICE.rexx
	@test -f ci/m9_2/AmBNC-AmBot-M9_2.cfg.in
	@test -f ci/m9_2/AmBot-via-AmBNC-M9_2.cfg
	@test -f ci/m9_2/AMBNC_INTEGRATION.md
	@test -f ci/m9_2/summarize-evidence.sh\n\t@test -f ci/m9_2/amigaos-template.fs-uae
	@echo "M9.2 harness static checks: PASS"

amiga: all
	@echo "AmBot native AmigaOS binary ready: $(TARGET)"

qualify-m9_2: all
	@sh ci/m9_2/prepare.sh


review-m9_2-evidence:
	@sh ci/m9_2/summarize-evidence.sh


run-m9_2-host:
	@bash ci/m9_2/run-host-fixture.sh
