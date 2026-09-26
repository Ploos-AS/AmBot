#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/../.." && pwd)
OUT="${M9_2_OUT:-$ROOT/build/m9_2}"
mkdir -p "$OUT/AmBot/M9_2" "$OUT/evidence"
cp "$ROOT/AmBot" "$OUT/AmBot/AmBot"
cp "$ROOT/examples/AmBot.cfg" "$OUT/AmBot/AmBot.cfg"
cp "$ROOT/examples/AmBot-Multi.cfg" "$OUT/AmBot/AmBot-Multi.cfg"
cp "$ROOT/examples/AmBot-Modern.cfg" "$OUT/AmBot/AmBot-Modern.cfg"
cp "$ROOT/examples/ambot.rexx" "$OUT/AmBot/ambot.rexx"
cp "$ROOT/examples/ON_PRIVMSG.rexx" "$OUT/AmBot/ON_PRIVMSG.rexx"
cp "$ROOT/ci/m9_2/AmBot-M9_2.cfg.in" "$OUT/AmBot/AmBot-M9_2.cfg.in"
cp "$ROOT/ci/m9_2/AmBNC-AmBot-M9_2.cfg.in" "$OUT/AmBot/AmBNC-AmBot-M9_2.cfg.in"
cp "$ROOT/ci/m9_2/AmBot-via-AmBNC-M9_2.cfg" "$OUT/AmBot/AmBot-via-AmBNC-M9_2.cfg"
cp "$ROOT/ci/m9_2/AMBNC_INTEGRATION.md" "$OUT/AmBot/AMBNC_INTEGRATION.md"
cp "$ROOT/ci/m9_2/rexx/"*.rexx "$OUT/AmBot/M9_2/"
cp "$ROOT/ci/m9_2/QUALIFICATION_CHECKLIST.md" "$OUT/evidence/QUALIFICATION_CHECKLIST.md"
if command -v sha256sum >/dev/null 2>&1; then sha256sum "$OUT/AmBot/AmBot" > "$OUT/evidence/AmBot.sha256"; fi
cat > "$OUT/README.txt" <<'EOF'
AmBot M9.2 local qualification bundle.
This directory contains only redistributable AmBot material.
Licensed AmigaOS/Kickstart files, credentials and generated configs stay local.
Run the checklist under evidence/ and retain visible runtime evidence locally.
EOF
echo "M9.2 bundle prepared at: $OUT"
echo "Qualification status remains PENDING until the visible AmigaOS run."
