# AmBot Roadmap

## M0 — Foundation — COMPLETE

- Repository and licensing baseline
- AmigaOS 2.04+ / 68000 target documented
- Bebbo GCC build baseline
- Module boundaries for IRC core, networking, events, commands and ARexx
- ARexx port name fixed as `AMBOT`
- Minimal native executable skeleton

## M1 — IRC session — IMPLEMENTED

- `bsdsocket.library` adapter
- TCP connect/disconnect
- IRC line framing/parser baseline
- PASS/NICK/USER registration
- PING/PONG
- reconnect with bounded backoff

Runtime qualification on classic Amiga/FS-UAE remains a separate qualification step; implementation is present in `main`.

## M2 — Event engine — IMPLEMENTED

- normalized IRC event model
- PRIVMSG/NOTICE/JOIN/PART/QUIT/KICK/TOPIC/NICK events
- shared dispatcher for built-ins and automation
- bounded event queue

Runtime qualification remains separate; see `docs/M2.md`.

## M3 — Bot commands — IMPLEMENTED

- command prefix handling
- command registry and dispatch
- sender/channel/private-message context
- USER/ADMIN permission model with optional owner nick
- built-in HELP, STATUS and VERSION commands

Runtime qualification remains separate; see `docs/M3.md`.

## M4 — ARexx command API — IMPLEMENTED

- live `AMBOT` message port
- STATUS, CONNECT, DISCONNECT, JOIN, PART, MSG, NOTICE, ACTION, WHOIS, MODE, TOPIC, RAW, RELOAD, QUIT
- shared internal operations used by native and ARexx callers
- ARexx and IRC socket activity integrated in one runtime loop with `WaitSelect()`

`RELOAD` is reserved and returns RC 5 until the configuration milestone. Runtime qualification remains separate; see `docs/M4.md`.

## M5 — ARexx event automation — IMPLEMENTED

- event-to-script hooks
- ON_PRIVMSG, ON_NOTICE, ON_JOIN, ON_PART, ON_QUIT, ON_KICK, ON_TOPIC, ON_NICK, ON_CONNECT, ON_DISCONNECT
- stable script argument contract
- return-code logging and hook failure isolation
- example hook script

Runtime qualification remains separate; see `docs/M5.md`.

## M6 — Persistence and modules

- configuration file
- persistent channel/network settings
- script/module discovery
- bounded per-module state where practical

## M7 — Multiple networks and AmBNC integration

- multiple IRC networks
- optional connection through AmBNC
- per-network command/script configuration

## M8 — Modern IRC extensions

- CAP negotiation
- SASL where feasible
- TLS strategy appropriate for classic Amiga hardware

## M9 — Qualification and release

- automated host/static qualification
- FS-UAE/AROS CI where practical
- local AmigaOS 2.04+ runtime qualification
- release packaging and documentation
