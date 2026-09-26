ifeq ($(origin CC), default)
CC := m68k-amigaos-gcc
endif
CFLAGS ?= -Os -Wall -Wextra -Werror -m68000 -Iinclude
LDFLAGS ?=
TARGET := AmBot
SOURCES := src/main.c src/net.c src/irc.c src/events.c src/commands.c src/operations.c src/rexx.c src/hooks.c src/config.c src/modules.c src/networks.c src/modernirc.c src/multinet.c src/session.c
OBJECTS := $(SOURCES:.c=.o)
HEADERS := include/ambot.h include/net.h include/irc.h include/events.h include/commands.h include/operations.h include/rexx.h include/hooks.h include/config.h include/modules.h include/networks.h include/modernirc.h include/multinet.h include/session.h

.PHONY: all clean check amiga qualify-m9_2 check-m9_2 preflight-m9_2 review-m9_2-evidence run-m9_2-host run-m9_2-tls check-m9_3 audit-m9_3 package-m9_3 prepare-publish-m9_3 publish-preflight-m9_3

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
	@python3 -m py_compile ci/m9_2/fixture_server.py ci/m9_2/tls_proxy.py ci/m9_2/tls_fixture.py
	@test -f ci/m9_2/AmBot-M9_2.cfg.in
	@test -f ci/m9_2/AmBot-SASL-Failure-M9_2.cfg.in
	@test -f ci/m9_2/AmBot-Permanent-Failure-M9_2.cfg.in
	@test -f ci/m9_2/rexx/Permanent-Failure-M9_2.rexx
	@test -f ci/m9_2/amiga/Permanent-Failure-M9_2
	@test -f ci/m9_2/rexx/Commands-M9_2.rexx
	@test -f ci/m9_2/rexx/ON_PRIVMSG.rexx
	@test -f ci/m9_2/rexx/ON_NOTICE.rexx
	@test -f ci/m9_2/rexx/SASL-Failure-M9_2.rexx
	@test -f ci/m9_2/amiga/SASL-Failure-M9_2
	@test -f ci/m9_2/AmBNC-AmBot-M9_2.cfg.in
	@test -f ci/m9_2/AmBNC-AmBot-TLS-M9_2.cfg.in
	@grep -Fq "TLS_MODE=PROXY" ci/m9_2/AmBNC-AmBot-TLS-M9_2.cfg.in
	@test -f ci/m9_2/AmBot-via-AmBNC-M9_2.cfg
	@test -f ci/m9_2/AMBNC_INTEGRATION.md
	@grep -Fq "typedef int (*ambot_irc_line_cb)" include/irc.h
	@grep -Fq "if (runtime->sock < 0) return 1;" src/multinet.c
	@test -f ci/m9_2/summarize-evidence.sh
	@test -f ci/m9_2/amigaos-template.fs-uae
	@echo "M9.2 harness static checks: PASS"

amiga: all
	@echo "AmBot native AmigaOS binary ready: $(TARGET)"

qualify-m9_2: all
	@sh ci/m9_2/prepare.sh


preflight-m9_2:
	@bash ci/m9_2/preflight.sh

review-m9_2-evidence:
	@sh ci/m9_2/summarize-evidence.sh


run-m9_2-host:
	@bash ci/m9_2/run-host-fixture.sh

run-m9_2-tls:
	@bash ci/m9_2/run-tls-transport.sh

check-m9_3:
	@bash -n ci/m9_3/release-gate.sh ci/m9_3/package-release.sh ci/m9_3/audit-release.sh ci/m9_3/prepare-publish.sh ci/m9_3/publish-preflight.sh
	@test -f ci/m9_3/RELEASE_NOTES.md.in
	@test -f ci/m9_3/audit-release.sh
	@test -f docs/M9_3_RELEASE.md
	@echo "M9.3 packaging static checks: PASS"

audit-m9_3: check-m9_3
	@bash ci/m9_3/audit-release.sh

package-m9_3: check-m9_3
	@bash ci/m9_3/package-release.sh

prepare-publish-m9_3: check-m9_3
	@test -n "$(VERSION)" || (echo "VERSION=vX.Y.Z required" >&2; exit 2)
	@bash ci/m9_3/prepare-publish.sh "$(VERSION)"

publish-preflight-m9_3: check-m9_3
	@test -n "$(VERSION)" || (echo "VERSION=vX.Y.Z required" >&2; exit 2)
	@bash ci/m9_3/publish-preflight.sh "$(VERSION)"
