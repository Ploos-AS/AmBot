#!/usr/bin/env bash
set -euo pipefail
out="${1:-build/m9_2}"
pass=0; pending=0; fail=0
r(){ printf '%-8s %s\n' "$1" "$2"; case "$1" in PASS) pass=$((pass+1));; FAIL) fail=$((fail+1));; *) pending=$((pending+1));; esac; }
need(){ if command -v "$1" >/dev/null 2>&1; then r PASS "$1 available"; else r FAIL "$1 required"; fi; }
echo "AmBot M9.2 preflight"
need python3
need openssl
python3 -m py_compile ci/m9_2/fixture_server.py ci/m9_2/tls_proxy.py ci/m9_2/tls_fixture.py && r PASS "Python qualification tools parse"
bash -n ci/m9_2/run-host-fixture.sh ci/m9_2/run-tls-transport.sh ci/m9_2/summarize-evidence.sh && r PASS "host shell tools parse"
sh -n ci/m9_2/prepare.sh && r PASS "bundle preparation parses"
if [[ -f ci/m9_2/AmBNC-AmBot-TLS-M9_2.cfg.in ]] && grep -Fq "TLS_MODE=PROXY" ci/m9_2/AmBNC-AmBot-TLS-M9_2.cfg.in && grep -Fq "PORT=17669" ci/m9_2/AmBNC-AmBot-TLS-M9_2.cfg.in; then r PASS "dedicated AmBNC TLS profile valid"; else r FAIL "dedicated AmBNC TLS profile invalid"; fi
if grep -R -F -n '\\n' ci/m9_2 Makefile --include='*.sh' --include='*.py' --include='Makefile' 2>/dev/null | grep -v 'rstrip("\\r\\n")' >/dev/null; then r FAIL "no accidental literal \\n escapes"; else r PASS "no accidental literal \\n escapes"; fi
if [[ -x AmBot || -f AmBot ]]; then r PASS "AmBot build artifact present"; else r PENDING "build AmBot before bundle preparation"; fi
if [[ -d "$out/AmBot" ]]; then r PASS "M9.2 bundle exists: $out"; else r PENDING "run make qualify-m9_2"; fi
manifest="$out/AmBot/evidence/MANIFEST.txt"
if [[ -f "$manifest" ]] && grep -Fq 'format=ambot-m9.2-evidence-v1' "$manifest" && grep -Fq 'binary_sha256=' "$manifest"; then r PASS "evidence manifest present"; else r PENDING "generate M9.2 evidence manifest"; fi
if [[ -f "$out/AmBot/AmBot-M9_2.cfg" ]]; then r PASS "direct runtime config materialized"; else r PENDING "run M9_2_HOST_IP=<host> make run-m9_2-host"; fi
if [[ -f "$out/AmBot/AmBot-SASL-Failure-M9_2.cfg" ]]; then r PASS "SASL failure config materialized"; else r PENDING "materialize SASL failure phase"; fi
if [[ -f "$out/AmBot/evidence/sasl-failure.txt" ]] && grep -Fq "network sasl-fail disabled after negotiation failure" "$out/AmBot/evidence/sasl-failure.txt"; then r PASS "SASL failure runtime evidence present"; else r PENDING "run visible SASL failure isolation phase"; fi
if [[ -f "$out/AmBot/AmBot-Permanent-Failure-M9_2.cfg" ]]; then r PASS "permanent failure config materialized"; else r PENDING "materialize permanent failure phase"; fi
if [[ -f "$out/AmBot/evidence/permanent-failure.txt" ]] && grep -Fq "M7 networks=1 skipped=1" "$out/AmBot/evidence/permanent-failure.txt"; then r PASS "permanent failure isolation evidence present"; else r PENDING "run visible permanent failure isolation phase"; fi
if [[ -f "$out/AmBot/evidence/ambnc.txt" ]]; then r PASS "AmBNC identity evidence present"; else r PENDING "record AmBot and AmBNC commit identities"; fi
if [[ -f "$out/AmBot/evidence/ambnc-plain.txt" ]] && grep -Fq "network via-ambnc connected" "$out/AmBot/evidence/ambnc-plain.txt"; then r PASS "real AmBNC plaintext transcript present"; else r PENDING "run visible AmBNC plaintext phase"; fi
if [[ -f "$out/AmBot/evidence/ambnc-tls.txt" ]] && grep -Fq "network via-ambnc-tls connected" "$out/AmBot/evidence/ambnc-tls.txt"; then r PASS "real AmBNC delegated TLS client transcript present"; else r PENDING "run visible AmBNC delegated TLS phase"; fi
if [[ -f "$out/AmBot/evidence/tls-proxy.jsonl" ]] && grep -Fq '"tls_version":' "$out/AmBot/evidence/tls-proxy.jsonl" && [[ -f "$out/AmBot/evidence/tls-fixture.jsonl" ]] && grep -Fq '"event": "tls_irc_registered"' "$out/AmBot/evidence/tls-fixture.jsonl" && grep -Fq '"event": "tls_irc_pong"' "$out/AmBot/evidence/tls-fixture.jsonl"; then r PASS "delegated TLS IRC evidence present"; else r PENDING "run make run-m9_2-tls with real AmBNC path"; fi
if [[ -f "$out/AmBot/evidence/Amiga-runtime-status.txt" ]]; then r PASS "AmigaOS snapshot evidence present"; else r PENDING "visible licensed AmigaOS run required"; fi
echo
echo "PASS=$pass PENDING=$pending FAIL=$fail"
[[ "$fail" -eq 0 ]]
