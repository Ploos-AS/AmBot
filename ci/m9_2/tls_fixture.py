#!/usr/bin/env python3
"""Deterministic TLS IRC endpoint for M9.2 delegated-transport evidence."""
import argparse, json, socketserver, ssl, time

def emit(event, **kw):
    r={"time":int(time.time()),"event":event}; r.update(kw)
    print(json.dumps(r,sort_keys=True),flush=True)

class S(socketserver.ThreadingMixIn,socketserver.TCPServer):
    allow_reuse_address=True; daemon_threads=True
    def get_request(self):
        s,a=super().get_request()
        tls=self.ctx.wrap_socket(s,server_side=True)
        emit("tls_fixture_client",tls_version=tls.version())
        return tls,a

class H(socketserver.StreamRequestHandler):
    def setup(self):
        super().setup(); self.nick=None; self.user=None; self.registered=False
    def send(self,line):
        self.wfile.write((line+"\r\n").encode()); self.wfile.flush()
    def maybe_register(self):
        if self.registered or not self.nick or not self.user: return
        self.registered=True
        self.send(":tls-fixture 001 %s :AmBot M9.2 TLS registered"%self.nick)
        self.send("PING :M9_2_TLS")
        emit("tls_irc_registered",nick=self.nick)
    def handle(self):
        self.send(":tls-fixture NOTICE AUTH :M9.2 TLS endpoint")
        for raw in self.rfile:
            line=raw.decode("utf-8","replace").rstrip("\r\n")
            cmd,_,args=line.partition(" "); cmd=cmd.upper()
            emit("tls_irc_rx",command=cmd)
            if cmd=="NICK":
                self.nick=args.lstrip(":").split()[0]; self.maybe_register()
            elif cmd=="USER":
                self.user=args.split()[0]; self.maybe_register()
            elif cmd=="PING":
                self.send("PONG "+args)
            elif cmd=="PONG":
                if "M9_2_TLS" in args: emit("tls_irc_pong",nick=self.nick or "unknown")
            elif cmd=="JOIN" and self.nick:
                ch=args.lstrip(":").split()[0]
                self.send(":%s!ambnc@tls-fixture JOIN :%s"%(self.nick,ch))
                emit("tls_irc_join",nick=self.nick,channel=ch)
            elif cmd=="QUIT":
                break

def main():
    p=argparse.ArgumentParser(); p.add_argument("--bind",default="127.0.0.1")
    p.add_argument("--port",type=int,default=17670); p.add_argument("--cert",required=True); p.add_argument("--key",required=True)
    a=p.parse_args(); ctx=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER); ctx.load_cert_chain(a.cert,a.key)
    s=S((a.bind,a.port),H); s.ctx=ctx; emit("tls_fixture_ready",port=a.port)
    try: s.serve_forever()
    except KeyboardInterrupt: pass
    finally: s.server_close()

if __name__=="__main__": main()
