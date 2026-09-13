#ifndef AMBOT_NET_H
#define AMBOT_NET_H

int ambot_net_open(void);
void ambot_net_close(void);
int ambot_net_connect_ipv4(const char *host, unsigned short port);
int ambot_net_send_all(int sock, const char *data, unsigned int length);
int ambot_net_recv(int sock, char *buffer, unsigned int length);
void ambot_net_close_socket(int sock);

#endif
