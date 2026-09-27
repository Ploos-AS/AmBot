#include <stdio.h>
#include <string.h>
#include "botai.h"

static const char *fixture;
static unsigned int fixture_pos;

int ambot_net_connect_ipv4(const char *host, unsigned short port) { (void)host; (void)port; fixture_pos=0; return 7; }
int ambot_net_send_all(int sock, const char *data, unsigned int length) { (void)sock; (void)data; (void)length; return 0; }
int ambot_net_recv(int sock, char *buffer, unsigned int length) {
    unsigned int n=0; (void)sock;
    while (fixture[fixture_pos] && n<length) buffer[n++]=fixture[fixture_pos++];
    return (int)n;
}
int ambot_net_wait_many_timed(const int *socks,unsigned int count,unsigned long mask,unsigned long *signals,unsigned long *ready,unsigned long timeout) {
    (void)socks;(void)count;(void)mask;(void)timeout; *signals=0; *ready=1; return 1;
}
void ambot_net_close_socket(int sock) { (void)sock; }

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); return 1; } } while(0)

int main(void) {
    struct ambot_botai b; char reply[128];
    CHECK(ambot_botai_configure(&b,"http://127.0.0.1:8090","auto",2)==0);
    CHECK(b.enabled && b.port==8090 && strcmp(b.host,"127.0.0.1")==0);
    CHECK(ambot_botai_configure(&b,"https://127.0.0.1","auto",2)!=0);
    CHECK(ambot_botai_configure(&b,"http://x:0","auto",2)!=0);
    CHECK(ambot_botai_configure(&b,"http://x:65536","auto",2)!=0);

    CHECK(ambot_botai_configure(&b,"http://x","auto",2)==0);
    fixture="HTTP/1.0 200 OK\r\n\r\n{\"api_version\":\"1.0.0\"}\n";
    CHECK(ambot_botai_check(&b)==0 && b.compatible);
    fixture="HTTP/1.0 200 OK\r\n\r\n{\"api_version\":\"2.0.0\"}\n";
    CHECK(ambot_botai_check(&b)!=0 && !b.compatible);

    fixture="HTTP/1.0 200 OK\r\n\r\n{\"api_version\":\"1.0.0\"}\n";
    CHECK(ambot_botai_check(&b)==0);
    fixture="HTTP/1.0 200 OK\r\n\r\n{\"expert\":\"general\",\"text\":\"hello world\",\"provider\":\"fixture\"}\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))==0);
    CHECK(strcmp(reply,"hello world")==0);
    fixture="HTTP/1.0 500 Error\r\n\r\n{\"error\":\"SECRET\"}\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))!=0 && reply[0]=='\0');
    fixture="HTTP/1.0 200 OK\r\n\r\n{not-json}\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))!=0);
    fixture="HTTP/1.0 200 OK\r\n\r\n{garbage \"text\":\"oops\"}\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))!=0);
    fixture="HTTP/1.0 200 OK\r\n\r\n{\"text\":\"first\",\"text\":\"second\"}\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))!=0);
    fixture="HTTP/1.0 200 OK\r\n\r\n{\"provider\":\"fixture\", \"text\" : \"hello\", \"expert\":\"general\"}\r\n";
    CHECK(ambot_botai_chat(&b,"hello",reply,sizeof(reply))==0 && strcmp(reply,"hello")==0);
    puts("BotAI host qualification: PASS");
    return 0;
}
