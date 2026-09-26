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

## Deterministic host fixture

`ci/m9_2/fixture_server.py` provides two local IRC endpoints. Alpha requires
CAP/SASL PLAIN and beta exercises a second simultaneous network. The fixture
redacts PASS and AUTHENTICATE payloads from its JSON-lines transcript.

Example host invocation:

```sh
python3 ci/m9_2/fixture_server.py --sasl-user m9user --sasl-pass "$AMBOT_M9_SECRET"
```

Copy `AmBot-M9_2.cfg.in` from the prepared bundle to the guest, replace
`HOST_IP` with the fixture host reachable from the Amiga and replace the
placeholder SASL password locally. Never commit the resulting config.

The direct fixture qualifies deterministic IRC/CAP/SASL and multi-network
behavior. The separate AmBNC-backed and TLS=UPSTREAM checklist rows still
require AmBNC in the path and must not be inferred from the direct fixture.
