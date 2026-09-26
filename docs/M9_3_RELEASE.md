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
release manifest. The archive receives its own SHA-256 sidecar. `ci/m9_3/audit-release.sh` is then run automatically and must PASS before the candidate is reported as prepared.

No ROM, AmigaOS/Workbench material, qualification credentials, private keys,
generated test configuration or local evidence is included.

The audit verifies the archive checksum, required files, release-manifest identity, packaged binary SHA-256, and rejects ROM/ADF/HDF, Workbench/Kickstart paths, private keys, credentials and local qualification/evidence material. It can also be rerun with `make audit-m9_3`.\n\nPackaging does not create a Git tag or GitHub Release. Publication remains an
explicit later gate after the candidate contents have been reviewed.

## Publication preparation

Publication remains intentionally separate from packaging. After M9.2 PASS and
a successful archive audit, run:

```sh
make prepare-publish-m9_3 VERSION=vX.Y.Z
make publish-preflight-m9_3 VERSION=vX.Y.Z
```

This renders release notes and `PUBLISH-MANIFEST.txt` with
`publish_status=NOT_PUBLISHED`. The preflight repeats the M9.2 gate and M9.3
archive audit, validates the requested version/tag identity and refuses an
already-existing local tag.

Neither target creates or pushes a Git tag or creates a GitHub Release.
