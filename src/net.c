#include <string.h>

#include <exec/libraries.h>
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/bsdsocket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>

#include "net.h"

struct Library *SocketBase = 0;

int ambot_net_open(void)
{
    if (SocketBase != 0) return 0;
    SocketBase = OpenLibrary((STRPTR)"bsdsocket.library", 0);
    return SocketBase != 0 ? 0 : -1;
}

void ambot_net_close(void)
{
    if (SocketBase != 0) {
        CloseLibrary(SocketBase);
        SocketBase = 0;
    }
}

int ambot_net_connect_ipv4(const char *host, unsigned short port)
{
    struct hostent *resolved;
    struct sockaddr_in address;
    int sock;

    resolved = gethostbyname((STRPTR)host);
    if (resolved == 0 || resolved->h_addr_list == 0 || resolved->h_addr_list[0] == 0)
        return -1;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    memcpy(&address.sin_addr, resolved->h_addr_list[0], sizeof(address.sin_addr));

    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        CloseSocket(sock);
        return -1;
    }
    return sock;
}

int ambot_net_send_all(int sock, const char *data, unsigned int length)
{
    unsigned int sent = 0;
    while (sent < length) {
        int rc = send(sock, (STRPTR)(data + sent), length - sent, 0);
        if (rc <= 0) return -1;
        sent += (unsigned int)rc;
    }
    return 0;
}

int ambot_net_recv(int sock, char *buffer, unsigned int length)
{
    return recv(sock, buffer, length, 0);
}

int ambot_net_wait(int sock, unsigned long signal_mask, unsigned long *signals)
{
    unsigned long ready = 0;
    int socks[1];
    socks[0] = sock;
    if (ambot_net_wait_many(socks, 1, signal_mask, signals, &ready) < 0) return -1;
    return (ready & 1UL) != 0 ? 1 : 0;
}

int ambot_net_wait_many(const int *socks,
                        unsigned int count,
                        unsigned long signal_mask,
                        unsigned long *signals,
                        unsigned long *ready_mask)
{
    fd_set readfds;
    ULONG signal_bits = (ULONG)signal_mask;
    int maxfd = -1;
    int rc;
    unsigned int i;
    unsigned long ready = 0;

    FD_ZERO(&readfds);
    for (i = 0; i < count; ++i) {
        if (socks[i] >= 0) {
            FD_SET(socks[i], &readfds);
            if (socks[i] > maxfd) maxfd = socks[i];
        }
    }

    rc = WaitSelect(maxfd + 1, &readfds, 0, 0, 0, &signal_bits);
    if (signals != 0) *signals = (unsigned long)signal_bits;
    if (rc < 0) return -1;

    for (i = 0; i < count && i < sizeof(unsigned long) * 8U; ++i)
        if (socks[i] >= 0 && FD_ISSET(socks[i], &readfds)) ready |= 1UL << i;

    if (ready_mask != 0) *ready_mask = ready;
    return rc;
}

void ambot_net_close_socket(int sock)
{
    if (sock >= 0) CloseSocket(sock);
}
