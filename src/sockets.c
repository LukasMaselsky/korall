#include "utils.h"
#include "sockets.h"

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

SOCKET socket_create(struct addrinfo* res) {
    return socket(res->ai_family, res->ai_socktype, res->ai_protocol);
}

/*
    To choose a port number, only needed if we're acting as a server
    If we're acting as a client connecting to remote, kernel will choose a local port for us so we don't need to call this
*/
int socket_bind(SOCKET sockfd, struct addrinfo* res) {
    return bind(sockfd, res->ai_addr, res->ai_addrlen);
}

int socket_reuse_port(SOCKET sockfd) {
    int opt = 1;
    return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int socket_connect(SOCKET sockfd, struct addrinfo* res) {
    return connect(sockfd, res->ai_addr, res->ai_addrlen);
}

int socket_listen(SOCKET sockfd) {
    return listen(sockfd, MAX_LISTEN_QUEUE_LEN);
}

/*
    While you are listenting on original socket, other device connects and placed into queue
    Accepting gets that incoming request and returns new fd for this connection only
    addr_len = sizeof(struct sockaddr_storage)
    addr_size = &addr_len // pointer because accept can change the length
*/
SOCKET socket_accept(SOCKET sockfd, struct sockaddr_storage* incoming, socklen_t* addr_size) {
    return accept(sockfd, (struct sockaddr*)incoming, addr_size);
}

/*
    Returns bytes sent
*/
int socket_send(SOCKET sockfd, const void* msg, int len, int flags) {
    return send(sockfd, msg, len, flags);
}

/*
    -1 = error
    0 = remote side closed connection
    other number = bytes read
*/
int socket_receive(SOCKET sockfd, void* buf, int len, int flags) {
    return recv(sockfd, buf, len, flags);
}

int socket_send_unconnected(SOCKET sockfd, const void* msg, int len, unsigned int flags,
    const struct sockaddr* to) {
    socklen_t tolen = sizeof(*to);
    return sendto(sockfd, msg, len, flags, to, tolen);
}

/*
    from becomes filled with originating ip and port
*/
int socket_receive_unconnected(SOCKET sockfd, void* buf, int len, unsigned int flags,
    struct sockaddr* from, int* fromlen) {
    return recvfrom(sockfd, buf, len, flags, from, fromlen);
}

// NOTE: send and recv can still be used with UDP if you do connect() (dest and src addresses added automatically)

/*
    Fills addr with info about other side of socket
*/
int socket_get_peer(SOCKET sockfd, struct sockaddr* addr, int* addrlen) {
    return getpeername(sockfd, addr, addrlen);
}

int get_host(char* hostname, size_t size) {
    return gethostname(hostname, size);
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

bool is_service_valid(const char* service) {
    if (service == NULL) {
        return false; // TODO: true?
    }
    int service_num;
    str_to_int_errno res = str_to_int(&service_num, service, 10);
    if (res == STR_TO_INT_SUCCESS) {
        if (service_num >= MIN_PORT_NUM && service_num <= MAX_PORT_NUM) {
            return true;
        }
    }

    if (strncmp(service, "http", strlen("http")) == 0 || strncmp(service, "https", strlen("https")) == 0) {
        return true;
    }

    return false;
}

// Either node or service, but not both, may be NULL.
int get_addr_info(
    const char* node,   // e.g. "www.example.com" or IP
    const char* service,  // e.g. "http" or port number
    const ProtocolFamily pf, // IPV4, IPV6, ANY
    const SocketType st, // TCP, UDP
    struct addrinfo** res
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

    int status = getaddrinfo(node, service, &hints, res);
    if (status != 0) {
        fprintf(stderr, "get_addr_info -> getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    return 0;
}

int get_addr_info_local(const char* port, struct addrinfo** res) {
    return get_addr_info(NULL, port, PF_ANY, TCP, res);
}

int get_addr_info_remote(const char* node, const char* service, struct addrinfo** res) {
    return get_addr_info(node, service, PF_ANY, TCP, res);
}


void print_addr_info(struct addrinfo* addrinfo) {
    void* addr;
    char* ipver;
    struct sockaddr_in* ipv4;
    struct sockaddr_in6* ipv6;
    char ipstr[INET6_ADDRSTRLEN];

    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    if (addrinfo->ai_family == AF_IPV4) {
        ipv4 = (struct sockaddr_in*)addrinfo->ai_addr;
        addr = &(ipv4->sin_addr);
        ipver = "IPv4";
    }
    else {
        ipv6 = (struct sockaddr_in6*)addrinfo->ai_addr;
        addr = &(ipv6->sin6_addr);
        ipver = "IPv6";
    }

    inet_ntop(addrinfo->ai_family, addr, ipstr, sizeof(ipstr));
    printf("%s: %s\n", ipver, ipstr);
}


#ifndef _WIN32
void sigchild_handler(int s) {
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}
#endif