#ifndef KORALL_SOCKET_H
#define KORALL_SOCKET_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include "korall/socket.h"
#include "http/tls/tls.h"


// https://stackoverflow.com/questions/28027937/cross-platform-sockets
#ifdef _WIN32

/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
/* 0x0600 https://stackoverflow.com/questions/60229778/inet-ntop-was-not-decleared-in-this-scope */
//#ifndef _WIN32_WINNT
//#define _WIN32_WINNT 0x0600
//#elif _WIN32_WINNT < 0x0600
//#undef _WIN32_WINNT
//#define _WIN32_WINNT 0x0600
//#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <synchapi.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib") // link to library
#pragma comment(lib, "IPHLPAPI.lib")

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
#include <ifaddrs.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INVALID_SOCKET (-1)

#endif


#define MAX_PORT_NUM_CHAR_LEN 5
#define MIN_PORT_NUM 1024
#define MAX_PORT_NUM 65535
#define IPV4_ADDRSTRLEN INET_ADDRSTRLEN - 1
#define IPV6_ADDRSTRLEN INET6_ADDRSTRLEN - 1
#define MAX_LISTEN_QUEUE_LEN 10
#define IP_VER_STR_LEN 5
#define SELECT_NO_TIMEOUT -1.0

typedef enum {
    AF_IPV4 = AF_INET,
    AF_IPV6 = AF_INET6,
    AF_ANY = AF_UNSPEC,
    AF_LOC = AF_UNIX
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

void socket_print(SOCKET sock);

SOCKET socket_create(struct addrinfo* res);

int socket_bind(SOCKET sockfd, struct addrinfo* res);

int socket_reuse_port(SOCKET sockfd);

int socket_connect(SOCKET sockfd, struct addrinfo* res);

int socket_listen(SOCKET sockfd);

SOCKET socket_accept(SOCKET sockfd, struct sockaddr_storage* incoming, socklen_t* addr_size);

int socket_send(SOCKET sockfd, const void* msg, int len, int flags);

int socket_send_secure(SOCKET sockfd, const void* msg, int len, int flags, SSL* ssl);

int socket_receive(SOCKET sockfd, void* buf, int len, int flags);

int socket_receive_secure(SOCKET sockfd, void* buf, int len, int flags, SSL* ssl);

int socket_send_unconnected(SOCKET sockfd, const void* msg, int len, unsigned int flags,
    const struct sockaddr* to);

int socket_receive_unconnected(SOCKET sockfd, void* buf, int len, unsigned int flags,
    struct sockaddr* from, int* fromlen);
    
int socket_get_peer(SOCKET sockfd, struct sockaddr* addr, int* addrlen);

int get_host(char* hostname, size_t size);

bool socket_creation_failed(SOCKET sock);

int socket_select_read_only(int nfds, fd_set* readfds, double timeout);

int socket_select_write_only(int nfds, fd_set* writefds, double timeout);

int socket_select_except_only(int nfds, fd_set* exceptfds, double timeout);

int socket_select_all(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, double timeout);

int get_addr_info_full(
    const char* node,   // e.g. "www.example.com" or IP
    const char* service,  // e.g. "http" or port number
    const ProtocolFamily pf, // IPV4, IPV6, ANY
    const SocketType st, // TCP, UDP
    struct addrinfo** res
);

int get_addr_info(const char* node, const char* service, struct addrinfo** res);

void get_ip_info_addr(struct addrinfo* addrinfo, char* ipstr, size_t ipstr_len, char* ipver, size_t ipver_len);

void get_ip_info_storage(struct sockaddr_storage* addrs, char* ipstr, size_t ipstr_len, char* ipver, size_t ipver_len);

void sigchild_handler(int s);

bool is_valid_port_num(const int port);

bool is_valid_port(char* port);

bool is_valid_service(char* service);

bool is_valid_ipv6(const char* ip);

bool is_valid_ipv4(const char* ip);

bool is_valid_ip(const char* ip);

int timeval_set(struct timeval* tv, double val);


#endif