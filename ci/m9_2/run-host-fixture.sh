#!/usr/bin/env bash
set -euo pipefail
out="${1:-build/m9_2}"
host="$out/host"
ev="$out/AmBot/evidence"
envf="$host/fixture.env"
mkdir -p "$host" "$ev"

if [[ ! -f "$envf" ]]; then
  umask 077
  secret="$(python3 -c 'import secrets; print(secrets.token_urlsafe(24))')"
  cat >"$envf" <<EOF
M9_2_BIND=0.0.0.0
M9_2_ALPHA_PORT=17667
M9_2_BETA_PORT=17668
M9_2_SASL_USER=m9user
M9_2_SASL_PASS=$secret
EOF
fi
# shellcheck disable=SC1090
source "$envf"

cfg="$out/AmBot/AmBot-M9_2.cfg"
if [[ ! -f "$out/AmBot/AmBot-M9_2.cfg.in" ]]; then
  echo "ERROR: run make qualify-m9_2 first" >&2; exit 2
fi
host_ip="${M9_2_HOST_IP:-}"
if [[ -z "$host_ip" ]]; then
  echo "ERROR: set M9_2_HOST_IP to the host address reachable from AmigaOS" >&2; exit 2
fi
sed -e "s|HOST_IP|$host_ip|g" -e "s|M9_2_SECRET|$M9_2_SASL_PASS|g"   "$out/AmBot/AmBot-M9_2.cfg.in" >"$cfg"
chmod 600 "$cfg"
fail_cfg="$out/AmBot/AmBot-SASL-Failure-M9_2.cfg"
if [[ ! -f "$out/AmBot/AmBot-SASL-Failure-M9_2.cfg.in" ]]; then
  echo "ERROR: missing SASL failure profile; run make qualify-m9_2 first" >&2; exit 2
fi
sed -e "s|HOST_IP|$host_ip|g" "$out/AmBot/AmBot-SASL-Failure-M9_2.cfg.in" >"$fail_cfg"
chmod 600 "$fail_cfg"

log="$ev/host-fixture.jsonl"
: >"$log"
python3 ci/m9_2/fixture_server.py --bind "$M9_2_BIND"   --alpha-port "$M9_2_ALPHA_PORT" --beta-port "$M9_2_BETA_PORT"   --sasl-user "$M9_2_SASL_USER" --sasl-pass "$M9_2_SASL_PASS" >>"$log" 2>&1 &
pid=$!
echo "$pid" >"$host/fixture.pid"
cleanup(){ kill "$pid" 2>/dev/null || true; wait "$pid" 2>/dev/null || true; rm -f "$host/fixture.pid"; }
trap cleanup EXIT INT TERM
sleep 1
if ! kill -0 "$pid" 2>/dev/null; then echo "ERROR: fixture failed to start" >&2; tail -20 "$log" >&2 || true; exit 1; fi

echo "M9.2 host fixture ready."
echo "  IRC evidence: $log"
echo "  Guest config: $cfg"
echo "  SASL failure phase: $fail_cfg"
echo "  Host address: $host_ip"
echo "Leave this running during the visible FS-UAE/AmigaOS test."
echo "Press Ctrl-C only after Snapshot-M9_2 and Shutdown-M9_2.rexx."
while kill -0 "$pid" 2>/dev/null; do sleep 1; done
echo "ERROR: fixture stopped unexpectedly" >&2
exit 1
