#include "utils.h"

/*
    Needed for Windows
*/
int socket_init(void) {
    #ifdef _WIN32
        WSADATA wsa_data;
        return WSAStartup(MAKEWORD(1, 1), &wsa_data);
    #else
        return 0;
    #endif
}

/*
    Needed for Windows
*/
int socket_quit(void) {
    #ifdef _WIN32
        return WSACleanup();
    #else
        return 0;
    #endif
}

int socket_close(SOCKET sock)
{
    int status = 0;

    #ifdef _WIN32
        status = shutdown(sock, SD_BOTH);
        if (status == 0) { status = closesocket(sock); }
    #else
        status = shutdown(sock, SHUT_RDWR);
        if (status == 0) { status = close(sock); }
    #endif

    return status;

}

bool socket_creation_failed(SOCKET sock) {
    bool failed = false;
        
    #ifdef _WIN32
        failed = sock == INVALID_SOCKET;
    #else
        failed = sock == -1;
    #endif

    return failed;
}