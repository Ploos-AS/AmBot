#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "$0")/../.." && pwd)
m92="${M9_2_OUT:-$root/build/m9_2}"
out="${M9_3_OUT:-$root/build/m9_3}"
"$root/ci/m9_3/release-gate.sh" "$m92"
rm -rf "$out"; mkdir -p "$out/AmBot/Examples" "$out/AmBot/Docs"
cp "$m92/AmBot/AmBot" "$out/AmBot/AmBot"
cp "$root/LICENSE" "$out/AmBot/LICENSE"
cp "$root/README.md" "$out/AmBot/README.md"
cp "$root/examples/"* "$out/AmBot/Examples/"
cp "$root/docs/ARCHITECTURE.md" "$out/AmBot/Docs/"
cp "$root/docs/M8.md" "$out/AmBot/Docs/"
commit=$(git -C "$root" rev-parse HEAD 2>/dev/null || echo unknown)
sha=$(sha256sum "$out/AmBot/AmBot" | awk '{print $1}')
cat >"$out/AmBot/RELEASE-MANIFEST.txt" <<EOF
format=ambot-release-v1
project=AmBot
source_commit=$commit
binary_sha256=$sha
baseline_cpu=68000
baseline_os=AmigaOS 2.04+
qualification=M9.2
qualification_status=PASS
license=MIT
EOF
( cd "$out" && tar -czf AmBot-m68k-amigaos.tar.gz AmBot )
sha256sum "$out/AmBot-m68k-amigaos.tar.gz" >"$out/AmBot-m68k-amigaos.tar.gz.sha256"
echo "M9.3 release candidate prepared: $out/AmBot-m68k-amigaos.tar.gz"
echo "Publication/tagging remains a separate explicit action."
