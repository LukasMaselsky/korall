#include <stdio.h>
#include "server/main.h"
#include "utils.h"

void sayHelloServer() {
	printf("Hello Server!\n");
}

int main() {
	
	socket_init(); 

	SOCKET sock = socket(IPV4, TCP, 0);
	if (socket_creation_failed(sock)) {
		perror("Socket creation failed");
		exit(EXIT_FAILURE);
	}

	printf("Opened socket\n");

	socket_close(sock);
	
	printf("Closed socket\n");

	socket_quit();

	return 0;
}