# AmBNC-backed M9.2 path

This gate uses a real AmBNC binary. It does not replace AmBNC with a protocol
mock.

1. Run the AmBot deterministic beta fixture on the host.
2. Start AmBNC on AmigaOS using `AmBNC-AmBot-M9_2.cfg.in` after replacing
   `HOST_IP` locally.
3. Confirm AmBNC's downstream listener is available on port 16668.
4. Start AmBot with `AmBot-via-AmBNC-M9_2.cfg`.
5. Confirm AmBot registers through the existing AmBNC upstream session, joins
   `#m9-beta`, responds to PING traffic, and remains usable after AmBot is
   restarted without restarting AmBNC.
6. Record both AmBot and AmBNC commit IDs in the checklist.

The plaintext profile uses `TLS=OFF` and listener 16668. The separate delegated-TLS profile uses `TLS=UPSTREAM` and listener 16669. In the TLS phase the upstream component is AmBNC; AmBot itself must not attempt a TLS handshake on the local AmBot-to-AmBNC connection.

For the separate external TLS proof, `tls_proxy.py` can be placed between an
AmBNC `TLS_MODE=PROXY` upstream and a TLS-enabled fixture. A negotiated TLS
version in its JSON-lines log is required evidence.

Do not commit generated configs, credentials, ROMs, Workbench files,
certificates/private keys, or licensed TCP/IP stack material.

## Deterministic delegated TLS phase

Run `make run-m9_2-tls` on the host. It creates an ignored, short-lived local
certificate, starts the TLS fixture on 17670 and a plaintext-to-TLS proxy on
17669. Use the dedicated `AmBNC-AmBot-TLS-M9_2.cfg.in` profile, replacing `HOST_IP` locally. It uses `TLS_MODE=PROXY`, host port 17669 and a separate downstream listener on 16669, so the plaintext integration profile remains unchanged.

Record `AmBot=<commit>` and `AmBNC=<commit>` in
`AMBOTQ:evidence/ambnc.txt`. The gate requires both identities plus a
`tls_proxy_connected` record containing the negotiated TLS version. The
certificate/private key remains under ignored `build/m9_2/host/tls`.


The TLS fixture is a minimal deterministic IRC server, not only a TLS socket.
The evidence review requires IRC registration and a response to
`PING :M9_2_TLS` through the delegated TLS path. A TLS handshake by itself is
therefore insufficient for PASS.

## Dedicated AmBot transcripts

Run `Execute AMBOTQ:AmBNC-Plain-M9_2` with the real plaintext AmBNC profile active, then drive shutdown from the second Shell with `RX AMBOTQ:M9_2/AmBNC-Plain-M9_2.rexx`. This writes `evidence/ambnc-plain.txt`.

For delegated TLS, keep `run-m9_2-tls` active, start real AmBNC with `AmBNC-AmBot-TLS-M9_2.cfg`, then run `Execute AMBOTQ:AmBNC-TLS-M9_2` and `RX AMBOTQ:M9_2/AmBNC-TLS-M9_2.rexx`. This writes `evidence/ambnc-tls.txt`. TLS PASS requires this client transcript, the proxy TLS version, and IRC registration/PONG at the TLS fixture.
