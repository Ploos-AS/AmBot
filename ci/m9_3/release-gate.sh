#!/usr/bin/env bash
set -euo pipefail
out="${1:-build/m9_2}"
ev="$out/AmBot/evidence"
status="$ev/Amiga-runtime-status.txt"
manifest="$ev/MANIFEST.txt"
die(){ echo "M9.3 RELEASE BLOCKED: $*" >&2; exit 1; }
[[ -f "$status" ]] || die "missing M9.2 Amiga runtime status"
grep -Fxq 'OVERALL=PASS' "$status" || die "M9.2 operator status is not OVERALL=PASS"
[[ -f "$manifest" ]] || die "missing M9.2 evidence manifest"
grep -Fq 'format=ambot-m9.2-evidence-v2' "$manifest" || die "wrong evidence manifest format"
run_id=$(sed -n 's/^run_id=//p' "$manifest")
commit=$(sed -n 's/^commit=//p' "$manifest")
expected=$(sed -n 's/^binary_sha256=//p' "$manifest")
[[ -n "$run_id" && "$run_id" != unknown ]] || die "missing qualification run identity"
[[ -n "$commit" && "$commit" != unknown ]] || die "missing qualified source commit"
grep -Fxq "RUN_ID=$run_id" "$status" || die "operator status belongs to a different qualification run"
grep -Fxq "COMMIT=$commit" "$status" || die "operator status source commit mismatch"
grep -Fxq "BINARY_SHA256=$expected" "$status" || die "operator status binary identity mismatch"
[[ -f "$out/AmBot/AmBot" ]] || die "qualified AmBot binary missing"
if command -v sha256sum >/dev/null 2>&1; then
  actual=$(sha256sum "$out/AmBot/AmBot" | awk '{print $1}')
  [[ -n "$expected" && "$expected" == "$actual" ]] || die "qualified binary SHA-256 mismatch"
else
  die "sha256sum is required"
fi
echo "M9.3 release gate: PASS"
