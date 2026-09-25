# AmBot

AmBot is a programmable IRC bot and automation engine for classic Amiga systems, built around first-class ARexx integration.

## Baseline

- Target: AmigaOS 2.04+
- CPU baseline: Motorola 68000
- Networking: `bsdsocket.library`
- ARexx port: `AMBOT`
- License: MIT
- Toolchain: Bebbo `m68k-amigaos-gcc`

## Current milestone: M9.2

M0-M8 are implemented and M9.1 automated AROS/FS-UAE qualification is PASS.
M9.2 now prepares local licensed-AmigaOS runtime qualification for networking,
ARexx, hooks, reload, multiple networks, AmBNC integration and CAP/SASL.
The milestone remains PENDING until the visible AmigaOS checklist is completed.

## Build

```sh
make
make check
make amiga
make qualify-m9_2
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
7. Keep PBMP/BotWeb support optional; AmBot must remain fully usable without a web control plane.

See `ROADMAP.md`, `docs/ARCHITECTURE.md` and `docs/M1.md` for the planned progression and M1 details.
