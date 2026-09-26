#!/usr/bin/env python3
"""Minimal TLS IRC endpoint for M9.2 delegated-transport evidence."""
import argparse, json, socketserver, ssl, time
def emit(event, **kw):
    r={"time":int(time.time()),"event":event}; r.update(kw); print(json.dumps(r,sort_keys=True),flush=True)
class S(socketserver.ThreadingMixIn,socketserver.TCPServer):
    allow_reuse_address=True; daemon_threads=True
    def get_request(self):
        s,a=super().get_request(); return self.ctx.wrap_socket(s,server_side=True),a
class H(socketserver.StreamRequestHandler):
    def handle(self):
        emit("tls_fixture_client")
        self.wfile.write(b":tls-fixture NOTICE AUTH :M9.2 TLS endpoint\r\n"); self.wfile.flush()
        for raw in self.rfile:
            line=raw.decode("utf-8","replace").rstrip("\r\n")
            cmd,_,args=line.partition(" ")
            if cmd.upper()=="PING":
                self.wfile.write(("PONG "+args+"\r\n").encode()); self.wfile.flush()
            if cmd.upper()=="QUIT": break
def main():
    p=argparse.ArgumentParser(); p.add_argument("--bind",default="127.0.0.1"); p.add_argument("--port",type=int,default=17670)
    p.add_argument("--cert",required=True); p.add_argument("--key",required=True); a=p.parse_args()
    ctx=ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER); ctx.load_cert_chain(a.cert,a.key)
    s=S((a.bind,a.port),H); s.ctx=ctx; emit("tls_fixture_ready",port=a.port)
    try:s.serve_forever()
    except KeyboardInterrupt:pass
    finally:s.server_close()
if __name__=="__main__": main()
