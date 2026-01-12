#include <stdio.h>
#include "client/main.h"
#include "utils.h"
#include "sockets.h"

#define MAX_BUFFER 200


int main(int argc, char *argv[]) {

	const char* node = argc > 1 ? argv[1] : NULL;
	const char* service = argc > 2 ? argv[2] : DEFAULT_PORT;

	SOCKET server_sock;
	struct addrinfo *serverinfo, *addrinfo;

	int err = socket_init();
	if (err != 0) {
		perror("Client: socket initialisation failed, exiting\n");
		exit(EXIT_FAILURE);
	}

	err = get_addr_info(node, service, &serverinfo);
	if (err != 0) {
		exit(EXIT_FAILURE);
	}

	// loop through all the results and bind to the first we can
	for (addrinfo = serverinfo; addrinfo != NULL; addrinfo = addrinfo->ai_next) {
		
		server_sock = socket_create(addrinfo);
		if (server_sock == -1) {
			perror("Client: socket");
			continue;
		}

		err = socket_connect(server_sock, addrinfo);
		if (err == -1) {
			perror("Client: connect");
			socket_close(server_sock);
			continue;
		}

		break;
	}

	if (addrinfo == NULL) {
		perror("Client: failed to connect");
		exit(EXIT_FAILURE);
	}

	char ip[IPV6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];
	get_ip_info_addr(addrinfo, ip, sizeof(ip), ipver, sizeof(ipver));
	printf("Client: connected to %s (%s)\n", ip, ipver);

	freeaddrinfo(serverinfo);

	char buffer[MAX_BUFFER];
	int num_bytes = socket_receive(server_sock, buffer, MAX_BUFFER-1, 0);
	if (num_bytes == -1) {
		perror("Client: couldn't read");
		exit(EXIT_FAILURE);
	}
	buffer[num_bytes] = '\0';

	printf("Client: received '%s'\n", buffer);

	socket_close(server_sock);

	socket_quit();
	
	return 0;
}