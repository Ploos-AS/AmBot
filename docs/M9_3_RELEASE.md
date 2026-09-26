# M9.3 — Release packaging — BLOCKED ON M9.2

M9.3 turns the exact M9.2-qualified AmBot binary into a redistributable release
candidate. It must never substitute a rebuild for the qualified binary.

## Gate

`ci/m9_3/release-gate.sh` requires:

- `evidence/Amiga-runtime-status.txt` containing exactly `OVERALL=PASS`
- the M9.2 evidence manifest with the expected format
- the qualified AmBot binary
- a SHA-256 match between that binary and the M9.2 manifest

Until those conditions are met, `make package-m9_3` fails closed.

## Package

The release candidate contains the qualified binary, MIT license, README,
redistributable examples, selected architecture/milestone documentation and a
release manifest. The archive receives its own SHA-256 sidecar.

No ROM, AmigaOS/Workbench material, qualification credentials, private keys,
generated test configuration or local evidence is included.

Packaging does not create a Git tag or GitHub Release. Publication remains an
explicit later gate after the candidate contents have been reviewed.
