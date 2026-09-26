#!/usr/bin/env bash
set -euo pipefail
version="${1:-}"
[[ "$version" =~ ^v[0-9]+\.[0-9]+\.[0-9]+([.-][0-9A-Za-z.-]+)?$ ]] || { echo "usage: $0 vX.Y.Z" >&2; exit 2; }
root=$(cd "$(dirname "$0")/../.." && pwd)
out="${M9_3_OUT:-$root/build/m9_3}"
"$root/ci/m9_3/release-gate.sh" "${M9_2_OUT:-$root/build/m9_2}"
M9_3_OUT="$out" "$root/ci/m9_3/audit-release.sh"
sed "s/@VERSION@/$version/g" "$root/ci/m9_3/RELEASE_NOTES.md.in" >"$out/RELEASE_NOTES.md"
cat >"$out/PUBLISH-MANIFEST.txt" <<EOF
format=ambot-publish-v1
version=$version
tag=$version
archive=AmBot-m68k-amigaos.tar.gz
checksum=AmBot-m68k-amigaos.tar.gz.sha256
release_notes=RELEASE_NOTES.md
publish_status=NOT_PUBLISHED
EOF
echo "M9.3 publish candidate prepared for $version"
echo "No tag or GitHub Release has been created."
