# M9.2 qualification checklist

Status: **PENDING**

Fill this file from a visible licensed AmigaOS 2.04+ run. Do not mark a row PASS
from static inspection or the M9.1 AROS run.

| Gate | Result | Evidence |
| --- | --- | --- |
| AmBot native launch | PENDING | |
| bsdsocket.library available | PENDING | |
| IRC registration/connect | PENDING | |
| forced disconnect/reconnect | PENDING | alpha forced_drop followed by connection 2 |
| AMBOT ARexx STATUS | PENDING | |
| ARexx JOIN / MSG / NOTICE | PENDING | |
| ON_PRIVMSG hook | PENDING | |
| hook failure isolation | PENDING | ON_NOTICE returns RC 10; later M9_2_AFTER_FAIL PRIVMSG is still handled |
| config reload | PENDING | |
| two simultaneous networks | PENDING | beta isolation_probe while alpha reconnects |
| AmBNC-backed network | PENDING | Record AmBot + AmBNC commits and fixture transcript |
| CAP LS 302 negotiation | PENDING | |
| SASL PLAIN success | PENDING | |
| SASL failure remains bounded/clean | PENDING | dedicated sasl-failure.txt shows negotiation failure + disable; fixture shows 904 and post-failure beta survivor probe |
| invalid security profile isolation | PENDING | dedicated phase shows M7 networks=1 skipped=1 and survivor connected |
| TLS=UPSTREAM via external proxy | PENDING | Record delegated AmBNC path and proxy TLS version |
| clean QUIT/shutdown | PENDING | |

## Required metadata

- AmBot commit:
- AmBot binary SHA-256:
- AmigaOS version:
- Machine / emulator:
- CPU:
- bsdsocket implementation:
- AmBNC version/commit:
- Host fixture revision:
- Date:
- Tester:

Licensed AmigaOS/Kickstart material and credentials must remain outside Git.

| real AmBNC plaintext path | PENDING | ambnc-plain.txt shows via-ambnc connected; AmBot/AmBNC commits recorded |
| real AmBNC delegated TLS path | PENDING | ambnc-tls.txt + TLS proxy version + tls_irc_registered + tls_irc_pong |
