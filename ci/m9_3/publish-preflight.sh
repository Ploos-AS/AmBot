#!/usr/bin/env bash
set -euo pipefail
version="${1:-}"
[[ -n "$version" ]] || { echo "usage: $0 vX.Y.Z" >&2; exit 2; }
root=$(cd "$(dirname "$0")/../.." && pwd)
out="${M9_3_OUT:-$root/build/m9_3}"
manifest="$out/PUBLISH-MANIFEST.txt"
die(){ echo "M9.3 PUBLISH BLOCKED: $*" >&2; exit 1; }
"$root/ci/m9_3/release-gate.sh" "${M9_2_OUT:-$root/build/m9_2}"
M9_3_OUT="$out" "$root/ci/m9_3/audit-release.sh"
[[ -f "$manifest" ]] || die "run prepare-publish first"
grep -Fxq "version=$version" "$manifest" || die "version mismatch"
grep -Fxq "tag=$version" "$manifest" || die "tag mismatch"
grep -Fxq 'publish_status=NOT_PUBLISHED' "$manifest" || die "unexpected publication state"
[[ -s "$out/RELEASE_NOTES.md" ]] || die "release notes missing"
if git -C "$root" rev-parse "$version" >/dev/null 2>&1; then die "tag already exists locally"; fi
echo "M9.3 publish preflight: PASS for $version"
echo "This command does not create or push a tag and does not create a GitHub Release."
