# AmBot

AmBot is a programmable IRC bot and automation engine for classic Amiga systems, built around first-class ARexx integration.

## M0 baseline

- Target: AmigaOS 2.04+
- CPU baseline: Motorola 68000
- Networking: `bsdsocket.library`
- ARexx port: `AMBOT`
- License: MIT
- Toolchain: Bebbo `m68k-amigaos-gcc`
- Initial scope: one IRC connection, command/event dispatch and ARexx automation foundations

M0 establishes the repository structure, architecture, build baseline and roadmap. Networking and live ARexx runtime behavior are introduced incrementally in later milestones.

## Build

```sh
make
```

Override the cross compiler if needed:

```sh
make CC=/opt/amiga/bin/m68k-amigaos-gcc
```

## Project layout

- `src/` — Amiga-native implementation
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

See `ROADMAP.md` and `docs/ARCHITECTURE.md` for the planned progression.
