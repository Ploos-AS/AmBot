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

`TLS=UPSTREAM` in AmBot means TLS is delegated to the upstream component.
For this gate the upstream component is AmBNC. AmBot itself must not attempt a
TLS handshake on the local 127.0.0.1:16668 connection.

For the separate external TLS proof, `tls_proxy.py` can be placed between an
AmBNC `TLS_MODE=PROXY` upstream and a TLS-enabled fixture. A negotiated TLS
version in its JSON-lines log is required evidence.

Do not commit generated configs, credentials, ROMs, Workbench files,
certificates/private keys, or licensed TCP/IP stack material.
