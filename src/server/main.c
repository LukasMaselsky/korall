#include "server/main.h"
#include "utils.h"
#include "sockets.h"

// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr


static int flag_str_to_val(char* key)
{
	for (FlagLookupEntry* entry = flag_lookup_table; entry != flag_lookup_table + NUM_OF_FLAGS; entry++) {
		if (strcmp(entry->key, key) == 0) {
			return entry->val;
		}
	}

	return F_BADFLAG;
}

static int process_flag(char* flag, Flags* flags) {

	int flag_val = flag_str_to_val(flag);

	switch (flag_val) {
		case F_TCP:
			flags->socktype = TCP;
			break;
		case F_UDP:
			flags->socktype = UDP;
			break;
		case F_BADFLAG:
		default:
			return -1;
	}

	return 0;
}


static int process_args(
	int argc,
	char* argv[],
	char* node,
	size_t max_node_len,
	char* service,
	size_t max_service_len,
	Flags* flags
) {
	// options:
	// 1. no ip no port (default ip (NULL), default port)
	// 2. ip without port (default port)
	// 3. no ip with port (default ip (NULL))
	// 4. ip and port

	int arg_count = argc - 1;

	if (arg_count == 0) {
		strncpy(service, DEFAULT_PORT, max_service_len);
		return 0;
	}

	for (int arg_i = 1; arg_i <= arg_count; arg_i++) {
		char* arg = argv[arg_i];

		if (arg_i == 1) {
			if (is_valid_ip(arg)) {
				strncpy(node, arg, max_node_len);
				strncpy(service, DEFAULT_PORT, max_service_len);
				continue;
			}

			if (is_valid_port(arg)) {
				strncpy(service, arg, max_service_len);
				continue;
			}

			if (process_flag(arg, flags) != -1) {
				strncpy(service, DEFAULT_PORT, max_service_len);
				continue;
			}

			printf("Not a valid port or IP address or flag\n");
			return -1;
		}

		if (arg_i == 2) {
			if (is_valid_ip(argv[1]) && is_valid_port(arg)) {
				strncpy(service, arg, max_service_len);
				continue;


				if (process_flag(arg, flags) != -1) {
					continue;
				}

				return -1;
			}

			if (arg_i < 3) continue; // safety 

			if (process_flag(arg, flags) != -1) {
				continue;
			}

			return -1;
		}
	}
	
	return 0;
}

int main(int argc, char *argv[]) {

	int err;

	Flags flags = { .socktype = DEFAULT_SOCK_TYPE };
	char node_arr[INET6_ADDRSTRLEN] = "\0";
	char service[MAX_PORT_NUM_CHAR_LEN];
	err = process_args(argc, argv, node_arr, INET6_ADDRSTRLEN, service, MAX_PORT_NUM_CHAR_LEN, &flags);
	if (err == -1) {
		printf("Could not process args");
		exit(EXIT_FAILURE);
	}
	char* node = strlen(node_arr) == 0 ? NULL : node_arr;

	SOCKET server_sock;
	struct addrinfo* serverinfo, *addrinfo;
	
	err = socket_init(); 
	if (err != 0) {
		perror("Server: socket initialisation failed, exiting");
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
			perror("Server: socket");
			continue;
		}

		int res = socket_reuse_port(server_sock);
		if (res == -1) {
			perror("Server: setsockopt");
			exit(EXIT_FAILURE);
		}

		res = socket_bind(server_sock, addrinfo);
		if (res == -1) {
			socket_close(server_sock);
			perror("Server: bind");
			continue;
		}

		break;
	}


	freeaddrinfo(serverinfo);


	if (addrinfo == NULL) {
		perror("Server: failed to bind");
		exit(EXIT_FAILURE);
	}

	char ip[INET6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];
	get_ip_info_addr(addrinfo, ip, sizeof(ip), ipver);
	printf("Server: opened socket on %s PORT %s (%s)\n", ip, service, ipver);

	int res = socket_listen(server_sock);
	if (res == -1) {
		perror("Server: socket listen\n");
		exit(EXIT_FAILURE);
	}


	// TODO: sigchld handler
	
	printf("Server: waiting for connections...\n");

	SOCKET incoming_sock;
	struct sockaddr_storage incoming_addr;
	socklen_t incoming_addr_len = sizeof(incoming_addr);
	char inc_ip[INET6_ADDRSTRLEN];
	char inc_ipver[IP_VER_STR_LEN];

	while (true) {
		incoming_sock = socket_accept(server_sock, &incoming_addr, &incoming_addr_len);
		if (incoming_sock == -1) {
			perror("Server: couldn't accept");
			continue;
		}

		get_ip_info_storage(&incoming_addr, inc_ip, sizeof(inc_ip), inc_ipver);
		printf("Server: got connection from %s (%s)\n", inc_ip, inc_ipver);

		char msg[13] = "Hello, world!";
		err = socket_send(incoming_sock, msg, 13, 0);
		if (err == -1) {
			perror("Server: couldn't send message");
		}

		socket_close(incoming_sock);

		break;
	}

	socket_close(server_sock);
	
	printf("Server: closed socket\n");

	socket_quit();

	return 0;
}
