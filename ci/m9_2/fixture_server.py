#!/usr/bin/env python3
"""Deterministic IRC fixture for AmBot M9.2. Never logs secrets."""
import argparse, base64, json, socketserver, threading, time

LOCK=threading.Lock()
CONNECTIONS={}

def emit(event, **fields):
    rec={"time":int(time.time()),"event":event}; rec.update(fields)
    with LOCK: print(json.dumps(rec,sort_keys=True),flush=True)

class Server(socketserver.ThreadingMixIn,socketserver.TCPServer):
    allow_reuse_address=True; daemon_threads=True
    def __init__(self,addr,network,user,password,require_sasl=False):
        self.network=network; self.sasl_user=user; self.sasl_pass=password
        self.require_sasl=require_sasl
        super().__init__(addr,Handler)

class Handler(socketserver.StreamRequestHandler):
    def setup(self):
        super().setup(); self.nick=None; self.user=None
        self.cap=False; self.cap_end=False; self.sasl_ok=not self.server.require_sasl
        self.registered=False
        with LOCK:
            CONNECTIONS[self.server.network]=CONNECTIONS.get(self.server.network,0)+1
            n=CONNECTIONS[self.server.network]
        emit("connected",network=self.server.network,connection=n)
    def send(self,line):
        self.wfile.write((line+"\r\n").encode()); self.wfile.flush()
    def maybe_register(self):
        if self.registered or not self.nick or not self.user: return
        if self.cap and not self.cap_end: return
        if not self.sasl_ok: return
        self.registered=True
        self.send(":fixture 001 %s :AmBot M9.2 registered"%self.nick)
        self.send("PING :M9_2_%s"%self.server.network.upper())
        emit("registered",network=self.server.network,nick=self.nick)
    def auth(self,arg):
        if arg=="PLAIN":
            self.send("AUTHENTICATE +"); emit("sasl_plain_requested",network=self.server.network); return
        try:
            got=base64.b64decode(arg.encode("ascii"),validate=True)
            exp=(self.server.sasl_user+"\0"+self.server.sasl_user+"\0"+self.server.sasl_pass).encode()
        except Exception: got=b""; exp=b"x"
        if got==exp:
            self.sasl_ok=True; self.send(":fixture 903 %s :SASL successful"%(self.nick or "*"))
            emit("sasl_pass",network=self.server.network)
        else:
            self.send(":fixture 904 %s :SASL failed"%(self.nick or "*"))
            emit("sasl_fail",network=self.server.network)
    def handle(self):
        self.send(":fixture NOTICE AUTH :AmBot M9.2 deterministic fixture")
        for raw in self.rfile:
            line=raw.decode("utf-8","replace").rstrip("\r\n")
            cmd,_,args=line.partition(" "); cmd=cmd.upper()
            emit("rx_redacted" if cmd in ("PASS","AUTHENTICATE") else "rx",
                 network=self.server.network,command=cmd,**({} if cmd in ("PASS","AUTHENTICATE") else {"line":line}))
            if cmd=="CAP" and args.upper().startswith("LS"):
                self.cap=True; self.send(":fixture CAP * LS :sasl"); emit("cap_ls",network=self.server.network)
            elif cmd=="CAP" and args.upper().startswith("REQ"):
                self.send(":fixture CAP * ACK :sasl"); emit("cap_ack",network=self.server.network)
            elif cmd=="CAP" and args.upper().startswith("END"):
                self.cap_end=True; self.maybe_register()
            elif cmd=="AUTHENTICATE": self.auth(args)
            elif cmd=="NICK": self.nick=args.lstrip(":").split()[0]; self.maybe_register()
            elif cmd=="USER": self.user=args.split()[0]; self.maybe_register()
            elif cmd=="PONG": emit("pong",network=self.server.network)
            elif cmd=="JOIN" and self.nick:
                ch=args.lstrip(":").split()[0]; self.send(":%s!ambot@fixture JOIN :%s"%(self.nick,ch))
                self.send(":fixture-user!test@fixture PRIVMSG %s :M9_2_HOOK"%ch); emit("join",network=self.server.network,channel=ch)
            elif cmd in ("PRIVMSG","NOTICE"):
                target,_,msg=args.partition(" "); emit(cmd.lower(),network=self.server.network,target=target,text=msg.lstrip(":"))
            elif cmd=="QUIT": break
    def finish(self):
        emit("disconnected",network=self.server.network,nick=self.nick or "unknown"); super().finish()

def main():
    p=argparse.ArgumentParser(); p.add_argument("--bind",default="0.0.0.0")
    p.add_argument("--alpha-port",type=int,default=17667); p.add_argument("--beta-port",type=int,default=17668)
    p.add_argument("--sasl-user",required=True); p.add_argument("--sasl-pass",required=True); a=p.parse_args()
    servers=[Server((a.bind,a.alpha_port),"alpha",a.sasl_user,a.sasl_pass,True),
             Server((a.bind,a.beta_port),"beta",a.sasl_user,a.sasl_pass,False)]
    for s in servers: threading.Thread(target=s.serve_forever,daemon=True).start()
    emit("fixture_ready",alpha_port=a.alpha_port,beta_port=a.beta_port)
    try:
        while True: time.sleep(1)
    except KeyboardInterrupt: pass
    for s in servers: s.shutdown(); s.server_close()

if __name__=="__main__": main()
