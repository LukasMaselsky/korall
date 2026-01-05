#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include <math.h>
#include <string.h>


/*
    Needed for Windows
*/
int socket_init(void) {
    #ifdef _WIN32
        printf("Windows platform detected, beginning startup\n");

        WSADATA wsa_data;
        WVersionInfo w_versions[VERSION_COUNT] = {
            GET_W_VERSION_INFO(2, 2),
            GET_W_VERSION_INFO(2, 0),
            GET_W_VERSION_INFO(1, 1),
            GET_W_VERSION_INFO(1, 0),
        };

        for (int i = 0; i < VERSION_COUNT; i++) {
            WVersionInfo w_version_info = w_versions[i];
            byte lobyte = w_version_info.lobyte;
            byte hibyte = w_version_info.hibyte;
            WORD w_version = w_version_info.w_version;
            printf("Trying v%u.%u\n", lobyte, hibyte);

            memset(&wsa_data, 0, sizeof(wsa_data));
            int err = WSAStartup(w_version, &wsa_data);
            if (err != 0) {
                printf("WSAStartup failed with error: %d\n", err);
                continue;
            }
            
            if (LOBYTE(wsa_data.wVersion) != lobyte || HIBYTE(wsa_data.wVersion) != hibyte) {
                printf("Could not find a usable version of Winsock.dll\n");
                WSACleanup();
                continue;
            }

            printf("Startup success!\n");
            return 0;
        }

        printf("Could not initialise any version of winsock");
        return 1;

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

int is_digits_only(const char* str) {
    // strspn returns the length of the initial segment of str consisting only of characters in 0123456789
    return strspn(str, "0123456789") == strlen(str);
}

bool is_service_valid(const char *service) {
    return strncmp(service, "http", strlen("http")) == 0 || strncmp(service, "https", strlen("https")) || is_digits_only(service);
}

// Either node or service, but not both, may be NULL.
int get_addr_info(
    const char* node,   // e.g. "www.example.com" or IP
    const char* service,  // e.g. "http" or port number
    const ProtocolFamily pf, // IPV4, IPV6, ANY
    const SocketType st, // TCP, UDP
    struct addrinfo* res
) {
    if (node == NULL && service == NULL) {
        perror("get_addr_info: both node and service cannot be null");
        return 1;
    }

    if (!is_service_valid(service)) {
        perror("get_addr_info: service is invalid");
        return 1;
    }
    // TODO: node format check e.g. 1.3.4.5 valid but 1...3.5 not valid

    
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = pf;
    hints.ai_socktype = st;
    if (node == NULL) {
        hints.ai_flags = AI_PASSIVE; // this machines IP
    }

    int status = getaddrinfo(node, service, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "get_addr_info -> getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    return 0;
}

void get_addr_info_local(const char *port, struct addrinfo *res) {
    get_addr_info(NULL, port, PF_ANY, TCP, res);
}

void get_addr_info_remote(const char *node, const char *service, struct addrinfo *res) {
    get_addr_info(node, service, PF_ANY, TCP, res);
}