# BotAI integration

AmBot supports BotAI v1 as an optional, fail-open adapter. BotAI is never required for IRC connectivity, ARexx, hooks, modules, persistence, multiple networks, AmBNC, CAP/SASL, or normal commands.

## Configuration

Global configuration keys:

- `BOTAI_URL` — enables the adapter when non-empty; currently `http://` only.
- `BOTAI_EXPERT` — expert id sent to BotAI; defaults to `auto`.
- `BOTAI_TIMEOUT` — bounded HTTP read timeout; defaults to 30 seconds.

The adapter checks `GET /v1/version` for BotAI API `1.0.0` before enabling AI commands. Failure or incompatibility leaves normal bot operation running.

## IRC command

`!AI <message>` sends one bounded message to `POST /v1/chat`. On service failure the user receives a generic temporary-unavailable response; provider/error bodies are not exposed to IRC.

The same adapter is passed to the single-network command context and every M7 multinet/AmBNC command context, including locally reconnected networks.

## Classic-Amiga constraints

The implementation is native C and uses the existing `bsdsocket.library` networking layer. It uses fixed-size buffers, no JSON library, and no heap allocation in the BotAI adapter. Responses are bounded and parsed as validated JSON objects rather than substring-matched.

## Qualification

Qualified commit: `60db7b837b8adc4866c1f3708e7cd7c236b6d60d`.

GitHub Actions run `36333105519` passed:

- host/static BotAI tests, including malformed/duplicate/reordered JSON and non-2xx fail-closed behavior;
- native Motorola 68000 Amiga build with Bebbo `m68k-amigaos-gcc`;
- AROS execution inside FS-UAE.

Result: **BotAI optional integration — PASS**.

This does not change the overall M9.2 status. M9.2 remains PENDING until its separate visible licensed-AmigaOS runtime checklist is completed.

## Known boundary

The current adapter is single-turn: it does not yet send BotAI conversation history. DNS and TCP connect use AmBot's existing blocking IPv4 connect path; the configured timeout bounds response waiting after connection.
