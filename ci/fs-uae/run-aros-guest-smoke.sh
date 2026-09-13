#!/usr/bin/env bash
set -euo pipefail
OUT_DIR="${1:-build/fs-uae/aros-guest}"
SYSTEM_DIR="build/fs-uae/aros-system"
mkdir -p "$OUT_DIR"
if [[ ! -f build/fs-uae/native/AmBot ]]; then echo 'ERROR: native AmBot binary missing' >&2; exit 1; fi
iso="$(ci/fs-uae/fetch-aros-system.sh "$SYSTEM_DIR" | tail -n 1)"
root_extract="$OUT_DIR/system-root"
rm -rf "$root_extract"; mkdir -p "$root_extract"
7z x -y -o"$root_extract" "$iso" >/dev/null
startup="$(find "$root_extract" -type f -ipath '*/s/startup-sequence' -print -quit)"
if [[ -z "$startup" ]]; then echo 'ERROR: AROS ISO lacks S/Startup-Sequence' >&2; exit 1; fi
aros_root="$(dirname "$(dirname "$startup")")"
cp build/fs-uae/native/AmBot "$aros_root/AmBot"
cp "$startup" "$startup.ambot-original"
cat > "$startup" <<'EOF'
SYS:C/Echo "M9_1_GUEST_STARTED=1" >SYS:ambot-m9.1-started.txt
SYS:C/Which AmBot >SYS:ambot-m9.1-which.txt
SYS:AmBot >SYS:ambot-m9.1-version.txt
SYS:C/Echo $RC >SYS:ambot-m9.1-rc.txt
SYS:C/Echo "M9_1_AFTER_AMBOT=1" >SYS:ambot-m9.1-after.txt
SYS:C/Execute SYS:S/Startup-Sequence.ambot-original
EOF
config="$OUT_DIR/aros-guest.fs-uae"
sed "s|@AROS_ROOT@|$PWD/$aros_root|" ci/fs-uae/aros-guest.fs-uae > "$config"
fs-uae --version > "$OUT_DIR/fs-uae-version.txt" 2>&1 || true
set +e
timeout 45s xvfb-run -a fs-uae "$config" > "$OUT_DIR/fs-uae.log" 2>&1
rc=$?
set -e
version_out="$aros_root/ambot-m9.1-version.txt"
status=FAIL
observation=guest_result_missing
if [[ -f "$version_out" ]] && grep -q 'AmBot 0.8.0-m8' "$version_out"; then status=PASS; observation=guest_executed_native_ambot; elif [[ -f "$aros_root/ambot-m9.1-after.txt" ]]; then observation=guest_executed_but_banner_mismatch; fi
{
 echo "STATUS=$status"
 echo "GATE=M9_1_AROS_GUEST_EXECUTION"
 echo "MODEL=A1200"
 echo "KICKSTART=internal"
 echo "FS_UAE_EXIT=$rc"
 echo "OBSERVATION=$observation"
 if [[ -f "$aros_root/ambot-m9.1-rc.txt" ]]; then tr -d '\r' < "$aros_root/ambot-m9.1-rc.txt" | sed 's/^/GUEST_RC=/'; fi
 if [[ -f "$version_out" ]]; then tr -d '\r' < "$version_out" | sed 's/^/GUEST_OUTPUT=/'; fi
} | tee "$OUT_DIR/result.txt"
[[ "$status" == PASS ]]
