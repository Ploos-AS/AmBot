#!/usr/bin/env bash
set -euo pipefail
out="${1:-build/m9_2}"
ev="$out/evidence"
fixture="$ev/host-fixture.jsonl"
proxy="$ev/tls-proxy.jsonl"
pass=0; pending=0; fail=0
result(){ printf '%-8s %s\n' "$1" "$2"; case "$1" in PASS) pass=$((pass+1));; FAIL) fail=$((fail+1));; *) pending=$((pending+1));; esac; }
event(){ if [[ -f "$fixture" ]] && grep -Fq "\"event\": \"$1\"" "$fixture" && grep -F "\"event\": \"$1\"" "$fixture"|grep -Fq "\"network\": \"$2\""; then result PASS "$3"; else result PENDING "$3"; fi; }
marker(){ if [[ -f "$ev/$1" ]] && grep -Fq "$2" "$ev/$1"; then result PASS "$3"; else result PENDING "$3"; fi; }

echo 'AmBot M9.2 evidence summary (advisory; operator checklist is authoritative)'
event fixture_ready all 'host fixture started'
event registered alpha 'alpha IRC registration'
event registered beta 'beta IRC registration'
event pong alpha 'alpha PING/PONG'
event pong beta 'beta PING/PONG'
event cap_ls alpha 'alpha CAP negotiation'
event sasl_pass alpha 'alpha SASL PLAIN'
event join alpha 'alpha channel join'
event join beta 'beta channel join'
marker rexx-commands.txt 'M9.2 STATUS RC=0' 'ARexx STATUS'
marker rexx-commands.txt 'M9.2 JOIN RC=0' 'ARexx JOIN'
marker rexx-commands.txt 'M9.2 MSG RC=0' 'ARexx MSG'
marker rexx-commands.txt 'M9.2 NOTICE RC=0' 'ARexx NOTICE'
marker rexx-commands.txt 'M9.2 RELOAD RC=0' 'ARexx RELOAD request'
if [[ -s "$ev/hooks.log" ]] && grep -Fq 'M9.2 HOOK' "$ev/hooks.log"; then result PASS 'ON_PRIVMSG hook evidence'; else result PENDING 'ON_PRIVMSG hook evidence'; fi
if [[ -f "$ev/ambnc.txt" ]] && grep -Fq 'AmBot=' "$ev/ambnc.txt" && grep -Fq 'AmBNC=' "$ev/ambnc.txt"; then result PASS 'AmBNC integration commit evidence'; else result PENDING 'AmBNC integration commit evidence'; fi
if [[ -f "$proxy" ]] && grep -Fq '"event": "tls_proxy_connected"' "$proxy" && grep -Fq '"tls_version":' "$proxy"; then result PASS 'external TLS transport evidence'; else result PENDING 'external TLS transport evidence'; fi

# Known fixture placeholder/secret strings must never appear in evidence logs.
if grep -R -F -q 'M9_2_SECRET' "$ev" 2>/dev/null; then result FAIL 'placeholder secret absent from evidence'; else result PASS 'placeholder secret absent from evidence'; fi

echo
echo "PASS=$pass PENDING=$pending FAIL=$fail"
echo 'OVERALL=PENDING'
echo 'Reason: licensed visible AmigaOS execution, environment identity, screenshots,'
echo 'hook-failure isolation and operator-reviewed checklist cannot be inferred here.'
[[ "$fail" -eq 0 ]]
