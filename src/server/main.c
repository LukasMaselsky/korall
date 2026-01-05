#include <stdio.h>
#include "server/main.h"
#include "utils.h"

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

int main() {
	
	int err = socket_init(); 
	if (err != 0) {
		perror("Socket initialisation failed, exiting");
		exit(EXIT_FAILURE);
	}

	return 0;

	AddressFamily af = AF_IPV4;

	SOCKET sock;
	switch (af) {
		case AF_IPV4:
			sock = ipv4();
			break;
		case AF_LOCAL:
			sock = local();
			break;
		case AF_IPV6:
		default:
			break;
	}


	printf("Opened socket\n");

	socket_close(sock);
	
	printf("Closed socket\n");

	socket_quit();

	return 0;
}
