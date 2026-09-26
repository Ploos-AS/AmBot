#!/usr/bin/env bash
set -euo pipefail
out="${1:-build/m9_2}"; host="$out/host"; ev="$out/AmBot/evidence"; tls="$host/tls"
mkdir -p "$tls" "$ev"

manifest="$ev/MANIFEST.txt"
[[ -f "$manifest" ]] || { echo "ERROR: missing M9.2 manifest; run make qualify-m9_2 first" >&2; exit 2; }
run_id=$(sed -n 's/^run_id=//p' "$manifest")
source_commit=$(sed -n 's/^commit=//p' "$manifest")
binary_sha=$(sed -n 's/^binary_sha256=//p' "$manifest")
[[ -n "$run_id" && "$run_id" != unknown && -n "$source_commit" && "$source_commit" != unknown && -n "$binary_sha" ]] || { echo "ERROR: incomplete M9.2 run identity" >&2; exit 2; }
identity_json=$(printf '{"event":"qualification_identity","run_id":"%s","commit":"%s","binary_sha256":"%s"}' "$run_id" "$source_commit" "$binary_sha")
if [[ ! -f "$tls/cert.pem" || ! -f "$tls/key.pem" ]]; then
  openssl req -x509 -newkey rsa:2048 -nodes -days 2 -subj '/CN=ambot-m9-2-fixture'     -keyout "$tls/key.pem" -out "$tls/cert.pem" >/dev/null 2>&1
  chmod 600 "$tls/key.pem"
fi
: >"$ev/tls-fixture.jsonl"; : >"$ev/tls-proxy.jsonl"
printf '%s\n' "$identity_json" >>"$ev/tls-fixture.jsonl"
printf '%s\n' "$identity_json" >>"$ev/tls-proxy.jsonl"
python3 ci/m9_2/tls_fixture.py --cert "$tls/cert.pem" --key "$tls/key.pem" >>"$ev/tls-fixture.jsonl" 2>&1 & fp=$!
python3 ci/m9_2/tls_proxy.py --bind 0.0.0.0 --listen-port 17669 --upstream-host 127.0.0.1 --upstream-port 17670 >>"$ev/tls-proxy.jsonl" 2>&1 & pp=$!
cleanup(){ kill "$fp" "$pp" 2>/dev/null || true; wait "$fp" "$pp" 2>/dev/null || true; }
trap cleanup EXIT INT TERM
sleep 1
kill -0 "$fp"; kill -0 "$pp"
echo "M9.2 TLS transport fixture ready on host port 17669."
echo "Configure real AmBNC TLS_MODE=PROXY upstream to <host>:17669."
echo "Leave running through the AmBNC-backed visible AmigaOS phase; Ctrl-C afterwards."
while kill -0 "$fp" 2>/dev/null && kill -0 "$pp" 2>/dev/null; do sleep 1; done
exit 1
