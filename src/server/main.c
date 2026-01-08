#include "server/main.h"
#include "utils.h"
#include "sockets.h"

static SOCKET ipv4() {
	// Create socket
	SOCKET sock = socket(AF_IPV4, TCP, 0);
	if (socket_creation_failed(sock)) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}
	

	//int opt = 1;
	// setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))

	// Bind socket
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr)); // zero the struct
	addr.sin_family = AF_IPV4;
	addr.sin_addr.s_addr = INADDR_ANY;

	

	return sock;
}

static SOCKET local() {
	// Create socket
	SOCKET sock = socket(AF_LOCAL, TCP, 0);
	if (socket_creation_failed(sock)) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	// Bind socket
	struct sockaddr* addr;
	memset(&addr, 0, sizeof(addr)); // zero the struct

}

#define PORT "3500"

void* get_in_addr(struct sockaddr* sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main() {

	SOCKET server_sock;
	struct addrinfo* serverinfo, *addrinfo;
	
	
	int err = socket_init(); 
	if (err != 0) {
		perror("Socket initialisation failed, exiting\n");
		exit(EXIT_FAILURE);
	}

	//err = get_addr_info_remote("www.example.net", NULL, &serverinfo);
	err = get_addr_info_local(PORT, &serverinfo);
	if (err != 0) {
		exit(EXIT_FAILURE);
	}

	// loop through all the results and bind to the first we can
	for (addrinfo = serverinfo; addrinfo != NULL; addrinfo = addrinfo->ai_next) {
		//	print_addr_info(addrinfo);
		server_sock = socket_create(addrinfo);
		if (server_sock == -1) {
			perror("server: socket\n");
			continue;
		}

		int res = socket_reuse_port(server_sock);
		if (res == -1) {
			perror("setsockopt\n");
			exit(EXIT_FAILURE);
		}

		res = socket_bind(server_sock, addrinfo);
		if (res == -1) {
			socket_close(server_sock);
			perror("server: bind\n");
			continue;
		}

		break;
	}


	freeaddrinfo(serverinfo);

	printf("Opened socket\n");

	if (addrinfo == NULL) {
		perror("server: failed to bind\n");
		exit(EXIT_FAILURE);
	}

	int res = socket_listen(server_sock);
	if (res == -1) {
		perror("socket listen\n");
		exit(EXIT_FAILURE);
	}


	// TODO: sigchld handler
	
	printf("Server: waiting for connections...\n");

	SOCKET incoming_sock;
	struct sockaddr_storage incoming_addr;
	socklen_t incoming_addr_len = sizeof(incoming_addr);
	char ip[INET6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];

	while (true) {
		incoming_sock = socket_accept(server_sock, &incoming_addr, &incoming_addr_len);
		if (incoming_sock == -1) {
			perror("Couldn't accept");
			continue;
		}

		get_ip_info_storage(&incoming_addr, ip, sizeof(ip), ipver);
		printf("Server: got connection from %s (%s)\n", ip, ipver);
		socket_close(incoming_sock);
		break;
	}

	socket_close(server_sock);
	
	printf("Closed socket\n");

	socket_quit();

	return 0;
}
