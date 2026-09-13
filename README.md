# AmBot

AmBot is a programmable IRC bot and automation engine for classic Amiga systems, built around first-class ARexx integration.

## Baseline

- Target: AmigaOS 2.04+
- CPU baseline: Motorola 68000
- Networking: `bsdsocket.library`
- ARexx port: `AMBOT`
- License: MIT
- Toolchain: Bebbo `m68k-amigaos-gcc`

## Current milestone: M1

M1 implements the first real IRC session layer:

- opens `bsdsocket.library`
- resolves IPv4 hosts with `gethostbyname()`
- TCP connect/disconnect
- bounded 512-byte IRC line framing
- optional PASS plus NICK/USER registration
- PING/PONG handling
- reconnect backoff from 1 second up to a 30-second ceiling
- Ctrl-C shutdown path

The current M1 runtime is deliberately small: one IRC server connection, no channel event engine yet, no bot command dispatcher yet, and no live ARexx port yet. Those are later milestones.

## Build

```sh
make
make check
```

Override the cross compiler if needed:

```sh
make CC=/opt/amiga/bin/m68k-amigaos-gcc
```

## Run

```text
AmBot HOST PORT NICK [USER] [PASS]
```

Example:

```text
AmBot irc.libera.chat 6667 AmBot
```

M1 uses plain TCP. TLS/SASL are intentionally deferred to the modern IRC milestone so the 68000/AmigaOS 2.04 baseline remains explicit.

## Project layout

- `src/net.c` — `bsdsocket.library` adapter
- `src/irc.c` — IRC framing and registration helpers
- `src/session.c` — upstream session, PING/PONG and reconnect loop
- `src/main.c` — CLI/bootstrap
- `include/` — public/internal headers
- `docs/` — architecture and milestone documentation
- `examples/` — ARexx examples as the interface grows

## Design principles

1. Keep the 68000/AmigaOS 2.04 baseline viable.
2. Treat ARexx as the primary extension and automation API.
3. Keep IRC protocol/state handling separate from Amiga-specific adapters.
4. Route IRC events through a shared dispatcher so built-ins and ARexx scripts see the same event model.
5. Prefer bounded memory structures suitable for classic hardware.
6. Keep AmBot interoperable with AmBNC without requiring it.

See `ROADMAP.md`, `docs/ARCHITECTURE.md` and `docs/M1.md` for the planned progression and M1 details.
