#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>

#ifdef _WIN32
/* See http://stackoverflow.com/questions/12765743/getaddrinfo-on-win32 */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501  /* Windows XP. */
#endif
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // link to library
#else
/* Assume that any non-Windows platform uses POSIX-style sockets instead. */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */

#define SOCKET int
#endif

#define IPV4 AF_INET
#define TCP SOCK_STREAM
#define UDP SOCK_DGRAM


int socket_init();

int socket_quit();

int socket_close(SOCKET sock);

bool socket_creation_failed(SOCKET sock);


#endif