#!/usr/bin/env bash
set -euo pipefail
out="${M9_3_OUT:-build/m9_3}"
archive="$out/AmBot-m68k-amigaos.tar.gz"
sidecar="$archive.sha256"
die(){ echo "M9.3 AUDIT FAIL: $*" >&2; exit 1; }
[[ -f "$archive" ]] || die "release archive missing"
[[ -f "$sidecar" ]] || die "archive SHA-256 sidecar missing"
( cd "$(dirname "$archive")" && sha256sum -c "$(basename "$sidecar")" ) >/dev/null || die "archive SHA-256 mismatch"
list=$(tar -tzf "$archive") || die "cannot read archive"
printf '%s\n' "$list" | grep -Fxq 'AmBot/AmBot' || die "binary missing"
printf '%s\n' "$list" | grep -Fxq 'AmBot/LICENSE' || die "license missing"
printf '%s\n' "$list" | grep -Fxq 'AmBot/RELEASE-MANIFEST.txt' || die "release manifest missing"
if printf '%s\n' "$list" | grep -Eiq '(^|/)(evidence|host|kickstart|workbench)(/|$)|\.(rom|adf|hdf|key|pem)$|\.fs-uae$|M9_2_SECRET|fixture\.env'; then
  die "forbidden or local qualification material present"
fi
tmp=$(mktemp -d); trap 'rm -rf "$tmp"' EXIT
tar -xzf "$archive" -C "$tmp"
manifest="$tmp/AmBot/RELEASE-MANIFEST.txt"
grep -Fxq 'format=ambot-release-v1' "$manifest" || die "wrong release manifest format"
grep -Fxq 'qualification=M9.2' "$manifest" || die "qualification identity missing"
grep -Fxq 'qualification_status=PASS' "$manifest" || die "release manifest is not qualified PASS"
expected=$(sed -n 's/^binary_sha256=//p' "$manifest")
actual=$(sha256sum "$tmp/AmBot/AmBot" | awk '{print $1}')
[[ -n "$expected" && "$expected" == "$actual" ]] || die "packaged binary SHA-256 mismatch"
if grep -R -E -i 'M9_2_SECRET|SASL_PASS=[^[:space:]]+|BEGIN (RSA |EC |OPENSSH )?PRIVATE KEY' "$tmp/AmBot" >/dev/null 2>&1; then
  die "credential/private-key pattern found in release contents"
fi
echo "M9.3 release audit: PASS"
