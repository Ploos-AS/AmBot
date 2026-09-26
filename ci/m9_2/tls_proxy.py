#!/usr/bin/env python3
"""Optional plain-listener to TLS-upstream relay for AmBot M9.2 evidence."""
import argparse, json, select, socket, socketserver, ssl, time
def emit(event,**kw):
    r={"time":int(time.time()),"event":event}; r.update(kw); print(json.dumps(r,sort_keys=True),flush=True)
class S(socketserver.ThreadingMixIn,socketserver.TCPServer):
    allow_reuse_address=True; daemon_threads=True
class H(socketserver.BaseRequestHandler):
    def handle(self):
        c=ssl.create_default_context(); c.check_hostname=False; c.verify_mode=ssl.CERT_NONE
        raw=socket.create_connection((self.server.host,self.server.port),timeout=10)
        up=c.wrap_socket(raw,server_hostname="ambot-m9-2-fixture")
        emit("tls_proxy_connected",tls_version=up.version())
        try:
            while True:
                rr,_,_=select.select([self.request,up],[],[],30)
                for src in rr:
                    data=src.recv(4096)
                    if not data:return
                    (up if src is self.request else self.request).sendall(data)
        finally:
            up.close(); emit("tls_proxy_disconnected")
def main():
    p=argparse.ArgumentParser(); p.add_argument("--bind",default="0.0.0.0"); p.add_argument("--listen-port",type=int,default=17669)
    p.add_argument("--upstream-host",default="127.0.0.1"); p.add_argument("--upstream-port",type=int,default=17670); a=p.parse_args()
    s=S((a.bind,a.listen_port),H); s.host=a.upstream_host; s.port=a.upstream_port
    emit("tls_proxy_ready",listen_port=a.listen_port,upstream_port=a.upstream_port)
    try:s.serve_forever()
    except KeyboardInterrupt:pass
    finally:s.server_close()
if __name__=="__main__":main()
