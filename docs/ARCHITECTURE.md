# Architecture

AmBot is split into small modules so the IRC protocol engine, event model and automation interfaces remain cleanly separated.

## Modules

- `core` — lifecycle and shared application state
- `irc` — IRC line parsing/formatting and protocol state
- `net` — `bsdsocket.library` integration
- `session` — IRC server connection/reconnect/session state
- `events` — normalized IRC events and dispatch
- `commands` — built-in bot command registry/dispatch
- `config` — configuration loading and validation
- `rexx` — ARexx message port, command API and event-script bridge

## ARexx

The public ARexx port is fixed as `AMBOT`. ARexx is a primary extension surface, not an optional compatibility layer.

The initial command vocabulary reserved for M4 is:

`STATUS`, `CONNECT`, `DISCONNECT`, `JOIN`, `PART`, `MSG`, `NOTICE`, `ACTION`, `WHOIS`, `MODE`, `TOPIC`, `RAW`, `RELOAD`, `QUIT`.

The event vocabulary reserved for M5 includes:

`ON_PRIVMSG`, `ON_NOTICE`, `ON_JOIN`, `ON_PART`, `ON_QUIT`, `ON_KICK`, `ON_TOPIC`, `ON_NICK`, `ON_CONNECT`, `ON_DISCONNECT`.

All externally callable operations should use shared core functions so built-ins, future UI paths and ARexx do not duplicate behavior.

## PBMP / BotWeb relationship

BotWeb is an optional external control plane. AmBot will implement the implementation-neutral PBMP/1 contract rather than embed a web server or BotWeb-specific UI code. The initial profile is limited to discovery and read-only status: `pbmp.info`, `capabilities.list`, `bot.info`, and `networks.list`.

PBMP message handling must remain independent of transport. LuCa and Engo can use local Unix sockets, but classic AmigaOS does not provide that same baseline. AmBot therefore uses an Amiga-appropriate local/bridge adapter while retaining identical PBMP messages. Network-exposed management is not enabled by default.

ARexx remains AmBot's primary native automation API. PBMP is a complementary management API for external control planes and must call shared core/status functions rather than duplicate IRC or ARexx behavior.

## AmBNC relationship

AmBot and AmBNC are sister projects. AmBot must work as a standalone IRC bot, but later milestones may allow AmBot to use AmBNC as its persistent upstream transport. Shared concepts should remain compatible without tightly coupling the two repositories.

## Constraints

The baseline target is a 68000-class Amiga running AmigaOS 2.04 or newer. Dynamic allocation, event queues and line buffers should remain bounded and predictable. Networking is implemented against `bsdsocket.library`; later protocol features must not silently raise the baseline requirements.
