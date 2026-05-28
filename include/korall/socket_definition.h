
#ifndef SOCKET_DEFINITION_H
#define SOCKET_DEFINITION_H

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

#define SOCKET int
#define SOCKET_ERROR -1
#endif

#endif