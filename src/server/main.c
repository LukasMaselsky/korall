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

int main() {

	SOCKET sock;
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
		print_addr_info(addrinfo);
		sock = socket_create(addrinfo);
		if (sock == -1) {
			perror("server: socket\n");
			continue;
		}

		int res = socket_reuse_port(sock);
		if (res == -1) {
			perror("setsockopt\n");
			exit(EXIT_FAILURE);
		}

		res = socket_bind(sock, addrinfo);
		if (res == -1) {
			socket_close(sock);
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

	int res = socket_listen(sock);
	if (res == -1) {
		perror("socket listen\n");
		exit(EXIT_FAILURE);
	}


	// TODO: sigchld handler
	
	printf("Server: waiting for connections...\n");

	while (true) {
		Sleep(1000);
	}

	socket_close(sock);
	
	printf("Closed socket\n");

	socket_quit();

	return 0;
}
