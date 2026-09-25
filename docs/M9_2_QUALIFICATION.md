# M9.2 — Local AmigaOS runtime qualification — PENDING

M9.2 qualifies the M8 feature set on licensed AmigaOS 2.04+ rather than the
internal AROS environment used by M9.1.

## Scope

A visible run must demonstrate all of the following before this milestone can
be marked PASS:

- native AmBot launch on the 68000 / AmigaOS 2.04+ baseline
- live bsdsocket.library IRC connect, disconnect and reconnect
- AMBOT ARexx port and representative command API calls
- ON_PRIVMSG and connection event hooks with failure isolation
- persistent configuration plus live RELOAD
- at least two simultaneous configured networks
- one direct network and one network using AmBNC integration
- IRCv3 CAP negotiation and SASL PLAIN against a controlled fixture
- TLS=UPSTREAM path through an external TLS terminator/proxy

## Evidence contract

Record the AmBot commit SHA, binary SHA-256, AmigaOS version, emulator or real
hardware identity, bsdsocket implementation, AmBNC commit/version where used,
and host-fixture version. Preserve console/transcript evidence for every row in
`ci/m9_2/QUALIFICATION_CHECKLIST.md`.

M9.1 is not sufficient evidence for these runtime rows. M9.2 remains PENDING
until the visible licensed-AmigaOS run is completed.

No Kickstart ROM, Workbench/AmigaOS files, credentials, SASL secrets or other
licensed material may be committed to this repository.
