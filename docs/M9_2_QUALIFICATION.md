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

## Evidence review

After the visible run has written its transcripts into `build/m9_2/evidence/`,
run:

```sh
make review-m9_2-evidence
```

The helper checks deterministic IRC registration, PING/PONG, CAP/SASL,
channel joins, representative ARexx results, hook evidence, AmBNC commit
evidence, TLS-proxy negotiation and basic secret redaction.

Its final result is deliberately always `OVERALL=PENDING`. A script cannot
attest that licensed AmigaOS 2.04+ was visibly running, identify the actual
TCP/IP stack, review screenshots, or judge the manual isolation/shutdown gates.
Any machine-detected FAIL must be resolved before an operator can mark M9.2
PASS.

## Operator sequence

Use three visible terminals/shells so host infrastructure and Amiga runtime
evidence remain distinct.

1. Run `make preflight-m9_2`, then build and prepare locally:
   `M9_2_SYSTEM_DIR=/licensed/system M9_2_KICKSTART_FILE=/licensed/kick.rom make qualify-m9_2`
2. On the host, start the deterministic fixture and leave it running:
   `M9_2_HOST_IP=<host-address-visible-to-AmigaOS> make run-m9_2-host`
3. Launch the generated `build/m9_2/AmBot-M9_2.fs-uae` visibly.
4. In the first AmigaShell run `AMBOTQ:Setup-M9_2`, then `AMBOTQ:Start-M9_2`.
5. In the second AmigaShell run `RX AMBOTQ:M9_2/Commands-M9_2.rexx` and capture
   its output to `AMBOTQ:evidence/rexx-commands.txt` as part of the visible
   procedure.
6. Exercise reconnect, hook failure isolation and the separate AmBNC-backed
   path described in `AMBNC_INTEGRATION.md`.
7. Run `AMBOTQ:Snapshot-M9_2`.
8. Run `RX AMBOTQ:M9_2/Shutdown-M9_2.rexx`.
9. Stop the host fixture with Ctrl-C, run `make preflight-m9_2`, then run `make review-m9_2-evidence`.

The host runner generates its SASL credential under ignored `build/m9_2/host`
with restrictive permissions and generates the guest config locally. Never
commit either generated file.
