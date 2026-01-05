#ifndef UTILS_H
#define UTILS_H
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

bool socket_creation_failed(SOCKET sock);


#endif