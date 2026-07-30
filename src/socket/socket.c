#include "utils/utils.h"
#include "socket/socket.h"
#include "http/tls/tls.h"

/*
    Needed for Windows
*/
int socket_init(void) {
#ifdef _WIN32
    log_msg(LOG_INFO, "windows platform detected, beginning startup\n");

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
        log_msg(LOG_INFO, "trying v%u.%u\n", lobyte, hibyte);

        memset(&wsa_data, 0, sizeof(wsa_data));
        int err = WSAStartup(w_version, &wsa_data);
        if (err != 0) {
            log_msg(LOG_WARN, "WSAStartup failed with error: %d\n", err);
            continue;
        }

        if (LOBYTE(wsa_data.wVersion) != lobyte || HIBYTE(wsa_data.wVersion) != hibyte) {
            log_msg(LOG_ERR, "could not find a usable version of Winsock.dll\n");
            WSACleanup();
            continue;
        }

        log_msg(LOG_INFO, "Startup success!\n");
        return 0;
    }

    log_msg(LOG_ERR, "could not initialise any version of winsock");
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

void socket_print(SOCKET sock) {
#ifdef _WIN32
    printf("%llu", sock);
#else
    printf("%d", sock);
#endif
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
    #ifdef _WIN32
        return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    #else
        return setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    #endif

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

int socket_send_secure(SOCKET sockfd, const void* msg, int len, int flags, SSL* ssl) {
    if (ssl == NULL) {
        return socket_send(sockfd, msg, len, flags);
    }

    return SSL_write(ssl, msg, len);
}

/*
    -1 = error
    0 = remote side closed connection
    other number = bytes read
*/
int socket_receive(SOCKET sockfd, void* buf, int len, int flags) {
    return recv(sockfd, buf, len, flags);
}

int socket_receive_secure(SOCKET sockfd, void* buf, int len, int flags, SSL* ssl) {
    if (ssl == NULL) {
        return socket_receive(sockfd, buf, len, flags);
    }
    return SSL_read(ssl, buf, len);
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



static int socket_select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, double timeout) {
    struct timeval tv;
    if (timeout != SELECT_NO_TIMEOUT) {
        timeval_set(&tv, timeout);
        return select(nfds, readfds, writefds, exceptfds, &tv);
    }
    else {
        return select(nfds, readfds, writefds, exceptfds, NULL);
    }

}

int socket_select_read_only(int nfds, fd_set* readfds, double timeout) {
    return socket_select(nfds, readfds, NULL, NULL, timeout);
}

int socket_select_write_only(int nfds, fd_set* writefds, double timeout) {
    return socket_select(nfds, NULL, writefds, NULL, timeout);
}

int socket_select_except_only(int nfds, fd_set* exceptfds, double timeout) {
    return socket_select(nfds, NULL, NULL, exceptfds, timeout);
}

int socket_select_all(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, double timeout) {
    return socket_select(nfds, readfds, writefds, exceptfds, timeout);
}


// Either node or service, but not both, may be NULL.
int get_addr_info_full(
    const char* node,   // e.g. "www.example.com" or IP
    const char* service,  // e.g. "http" or port number
    const ProtocolFamily pf, // IPV4, IPV6, ANY
    const SocketType st, // TCP, UDP
    struct addrinfo** res
) {
    if (node == NULL && service == NULL) {
        log_msg(LOG_ERR, "get_addr_info: both node and service cannot be null");
        return 1;
    }


    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints)); // make sure the struct is empty
    hints.ai_family = pf;
    hints.ai_socktype = st;
    if (node == NULL) {
        // hints.ai_flags = AI_PASSIVE; // this machines IP
        //! When AI_PASSIVE, can't use NULL for node (doesn't connect on client even though 0.0.0.0 assigned)
        //! todo
    }

    int status = getaddrinfo(node, service, &hints, res);
    if (status != 0) {
        log_msg(LOG_ERR, "get_addr_info -> getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    return 0;
}

/*
    node = url / ip
    service = http / port num / etc
*/
int get_addr_info(const char* node, const char* service, struct addrinfo** res) {
    return get_addr_info_full(node, service, PF_ANY, TCP, res);
}


void get_ip_info_addr(struct addrinfo* addrinfo, char* ipstr, size_t ipstr_len, char* ipver, size_t ipver_len) {
    void* addr;
    struct sockaddr_in* ipv4;
    struct sockaddr_in6* ipv6;

    // get the pointer to the address itself,
    // different fields in IPv4 and IPv6:
    if (addrinfo->ai_family == AF_IPV4) {
        ipv4 = (struct sockaddr_in*)addrinfo->ai_addr;
        addr = &(ipv4->sin_addr);
        strncpy(ipver, "IPv4", ipver_len);
    }
    else {
        ipv6 = (struct sockaddr_in6*)addrinfo->ai_addr;
        addr = &(ipv6->sin6_addr);
        strncpy(ipver, "IPv6", ipver_len);
    }

    inet_ntop(addrinfo->ai_family, addr, ipstr, ipstr_len);
}

void get_ip_info_storage(struct sockaddr_storage* addrs, char* ipstr, size_t ipstr_len, char* ipver, size_t ipver_len) {
    void* addr;
    struct sockaddr_in* ipv4;
    struct sockaddr_in6* ipv6;

    struct sockaddr* sa = (struct sockaddr*)addrs;

    if (sa->sa_family == AF_IPV4) {
        ipv4 = (struct sockaddr_in*)sa;
        addr = &(ipv4->sin_addr);
        strncpy(ipver, "IPv4", ipver_len);
    }
    else {
        ipv6 = (struct sockaddr_in6*)sa;
        addr = &(ipv6->sin6_addr);
        strncpy(ipver, "IPv6", ipver_len);
    }

    inet_ntop(sa->sa_family, addr, ipstr, ipstr_len);
}

bool is_valid_port_num(const int port) {
    return (port >= MIN_PORT_NUM && port <= MAX_PORT_NUM);
}

bool is_valid_port(char* port) {
    int service_num;
    str_to_int_errno res = str_to_int(&service_num, port, 10);
    if (res != STR_TO_INT_SUCCESS) return false;

    return is_valid_port_num(service_num);
}

bool is_valid_service(char* service) {
    if (is_valid_port(service)) {
        return true;
    }

    if (strncmp(service, "http", strlen("http")) == 0 || strncmp(service, "https", strlen("https")) == 0) {
        return true;
    }
    return false;
}

bool is_valid_ipv4(const char* ip) {
    char buf[IPV6_ADDRSTRLEN + 1] = { 0 };
    return inet_pton(AF_IPV4, ip, buf);
}

bool is_valid_ipv6(const char* ip) {
    char buf[IPV6_ADDRSTRLEN + 1] = { 0 };
    return inet_pton(AF_IPV6, ip, buf);
}

bool is_valid_ip(const char* ip) {
    // allow "localhost"
    return is_valid_ipv4(ip) || is_valid_ipv6(ip) || strcmp(ip, "localhost") == 0;
}

int timeval_set(struct timeval *tv, double val) {
    if (val < 0) return -1;
    
    int tv_sec = (int)val;
    double decimal = val - tv_sec;
    double usec_d = decimal * 1000000;
    int tv_usec = (int)usec_d;
    
    tv->tv_sec = tv_sec;
    tv->tv_usec = tv_usec;
    return 0;
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
