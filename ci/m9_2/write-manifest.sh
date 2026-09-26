#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
OUT="${M9_2_OUT:-$ROOT/build/m9_2}"
EV="$OUT/AmBot/evidence"
BIN="$OUT/AmBot/AmBot"
mkdir -p "$EV"
commit=unknown
if command -v git >/dev/null 2>&1 && git -C "$ROOT" rev-parse --verify HEAD >/dev/null 2>&1; then commit=$(git -C "$ROOT" rev-parse HEAD); fi
run_id=unknown
if command -v date >/dev/null 2>&1; then
  run_id="$(date -u +%Y%m%dT%H%M%SZ)-$(printf '%s' "$commit" | cut -c1-12)"
fi
sha=unavailable
if command -v sha256sum >/dev/null 2>&1 && [ -f "$BIN" ]; then sha=$(sha256sum "$BIN" | awk '{print $1}'); fi
cat >"$EV/MANIFEST.txt" <<EOF
format=ambot-m9.2-evidence-v2
run_id=$run_id
project=AmBot
commit=$commit
binary_sha256=$sha
qualification=M9.2
overall=PENDING
phase.direct_irc=required
phase.multinet_reconnect=required
phase.arexx_commands=required
phase.hook_failure_isolation=required
phase.sasl_success=required
phase.sasl_failure_isolation=required
phase.permanent_security_isolation=required
phase.ambnc=required
phase.delegated_tls=required
phase.visible_amigaos=required
EOF
echo "M9.2 evidence manifest written: $EV/MANIFEST.txt"
