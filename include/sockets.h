#ifndef SOCKETS_H
#define SOCKETS_H
#include <stdbool.h>
// https://stackoverflow.com/questions/28027937/cross-platform-sockets
#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // link to library

#define VERSION_COUNT 4

typedef struct {
    WORD w_version;
    byte lobyte;
    byte hibyte;
} WVersionInfo;


#define GET_W_VERSION_INFO(lo, hi) {MAKEWORD(lo, hi), lo, hi}

#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */

#define SOCKET int
#define SOCKET_ERROR -1
#endif

#define MAX_PORT_NUM_CHAR_LEN 5
#define MIN_PORT_NUM 1024
#define MAX_PORT_NUM 65535
#define IPV4_ADDRSTRLEN INET_ADDRSTRLEN
#define IPV6_ADDRSTRLEN INET6_ADDRSTRLEN
#define MAX_LISTEN_QUEUE_LEN 10

typedef enum {
    AF_IPV4 = AF_INET,
    AF_IPV6 = AF_INET6,
    AF_LOCAL = AF_UNIX
} AddressFamily;

typedef enum {
    PF_IPV4 = PF_INET,
    PF_IPV6 = PF_INET6,
    PF_ANY = PF_UNSPEC
} ProtocolFamily;

typedef enum {
    UNSPEC = 0,
    TCP = SOCK_STREAM,
    UDP = SOCK_DGRAM,
} SocketType;

int socket_init();

int socket_quit();

int socket_close(SOCKET sock);

SOCKET socket_create(struct addrinfo* res);

int socket_bind(SOCKET sockfd, struct addrinfo* res);

int socket_reuse_port(SOCKET sockfd);

int socket_connect(SOCKET sockfd, struct addrinfo* res);

int socket_listen(SOCKET sockfd);

SOCKET socket_accept(SOCKET sockfd, struct sockaddr_storage* incoming, socklen_t* addr_size);

int socket_send(SOCKET sockfd, const void* msg, int len, int flags);

int socket_receive(SOCKET sockfd, void* buf, int len, int flags);

int socket_send_unconnected(SOCKET sockfd, const void* msg, int len, unsigned int flags,
    const struct sockaddr* to);

int socket_receive_unconnected(SOCKET sockfd, void* buf, int len, unsigned int flags,
    struct sockaddr* from, int* fromlen);
    
int socket_get_peer(SOCKET sockfd, struct sockaddr* addr, int* addrlen);

int get_host(char* hostname, size_t size);

bool socket_creation_failed(SOCKET sock);

bool is_service_valid(const char* service);

int get_addr_info(
    const char* node,   // e.g. "www.example.com" or IP
    const char* service,  // e.g. "http" or port number
    const ProtocolFamily pf, // IPV4, IPV6, ANY
    const SocketType st, // TCP, UDP
    struct addrinfo* res
);

int get_addr_info_local(const char* port, struct addrinfo* res);

int get_addr_info_remote(const char* node, const char* service, struct addrinfo* res);


#endif