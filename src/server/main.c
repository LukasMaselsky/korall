#include "server/main.h"
#include "utils.h"
#include "sockets.h"
#include "http_.h"
#include "lookup_tables.h"


// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr

static Flag flag_str_to_val(char* key)
{
	// check for "--"
	for (int i = 0; i < 2; i++) {
		char c = key[0];
		if (c == '\0') return F_BADFLAG;
		if (c != '-') return F_BADFLAG;
		key++;
	}
	// + 1 extra char at least ("--" only invalid)
	if (key[0] == '\0') return F_BADFLAG;

	int val = lookup(key, flag_lookup_table, FLAG_LOOKUP_TABLE_COUNT, true);
	if (val == -1) {
		return F_BADFLAG;
	}
	return val;
}

static int process_flag(char* flag, Flags* flags) {

	int flag_val = flag_str_to_val(flag);

	switch (flag_val) {
		case F_TCP:
			flags->server_type = ST_TCP;
			break;
		case F_HTTP:
			flags->server_type = ST_HTTP;
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


static void process_http_request(
	SOCKET inc_sock, 
	SOCKET server_sock, 
	const char* data, 
	int data_len, 
	fd_set* main, 
	SOCKET fd_max
) {

	HTTPRequest *req = http_request_st_init();

	if (validate_http_request(data, data_len, req) == -1) {
		// send invalid response
		printf("INVALID\n");
		return;
	}
	printf("VALID\n");
	// send response;

	http_request_st_free(req);

	return;
}

/*
	Initialise a socket for listening
*/
SOCKET init_listen_socket(const char* node, const char* service) {
	int res;
	SOCKET sock;
	struct addrinfo *serverinfo, *addrinfo;


	if (node == NULL || strcmp(node, "localhost") == 0) {
		node = LOCALHOST_NODE; // default server to localhost
	}
	res = get_addr_info(node, service, &serverinfo);
	
	if (res != 0) {
		exit(EXIT_FAILURE);
	}

	// loop through all the results and bind to the first we can
	for (addrinfo = serverinfo; addrinfo != NULL; addrinfo = addrinfo->ai_next) {
		sock = socket_create(addrinfo);
		if (sock == -1) {
			perror("server: socket");
			continue;
		}

		res = socket_reuse_port(sock);
		if (res == -1) {
			perror("server: setsockopt");
			exit(EXIT_FAILURE);
		}

		res = socket_bind(sock, addrinfo);
		if (res == -1) {
			socket_close(sock);
			perror("server: bind");
			continue;
		}

		break;
	}


	freeaddrinfo(serverinfo);


	if (addrinfo == NULL) {
		perror("server: failed to bind");
		exit(EXIT_FAILURE);
	}

	char ip[IPV6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];
	get_ip_info_addr(addrinfo, ip, sizeof(ip), ipver, sizeof(ipver));
	printf("server: opened socket on %s PORT %s (%s)\n", ip, service, ipver);

	res = socket_listen(sock);
	if (res == -1) {
		perror("server: socket listen\n");
		exit(EXIT_FAILURE);
	}

	return sock;
}

void process_incoming_connection(SOCKET sock, fd_set* main, SOCKET* fd_max) {
	SOCKET incoming;
	struct sockaddr_storage incoming_addr;
	socklen_t incoming_addr_len = sizeof(incoming_addr);
	char ip[IPV6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];

	
	incoming = socket_accept(sock, &incoming_addr, &incoming_addr_len);
	if (incoming == -1) {
		perror("server: couldn't accept");
		return;
	}

	FD_SET(incoming, main); // add fd to set
	if (incoming > *fd_max) {
		*fd_max = incoming;
	}
	get_ip_info_storage(&incoming_addr, ip, sizeof(ip), ipver, sizeof(ipver));
	printf("server: got connection from %s (%s)\n", ip, ipver);
}

void broadcast(SOCKET inc_sock, SOCKET server_sock, const char* data, int data_len, fd_set* main, SOCKET fd_max) {
	// send data received to every other connection except incoming and server
	for (SOCKET fd = 0; fd <= fd_max; fd++) {
		if (!FD_ISSET(fd, main)) continue;
			
		if (fd == server_sock || fd == inc_sock) continue;
			
		int res = socket_send(fd, data, data_len, 0);
		if (res == -1) {
			printf("server: couldn't send data to ");
			socket_print(fd);
			printf("\n");
		}
	}
}

void process_incoming_data(SOCKET inc_sock, SOCKET server_sock, fd_set* main, SOCKET fd_max, Flags* flags) {
	char buffer[READ_BUFFER_LEN];    // buffer for client data

	int bytes_read = socket_receive(inc_sock, buffer, READ_BUFFER_LEN - 1, 0);
	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			printf("server: socket ");
			socket_print(inc_sock);
			printf(" closed connection\n");
		}
		else {
			printf("server: couldn't read from ");
			socket_print(inc_sock);
			printf("\n");
		}

		socket_close(inc_sock);
		FD_CLR(inc_sock, main); // remove from set
		return;
	}

	buffer[bytes_read] = '\0';
	printf("server: received data from ");
	socket_print(inc_sock);
	printf("\n'%s'\n", buffer);

	// TODO
	if (flags->server_type == ST_HTTP) {
		process_http_request(inc_sock, server_sock, buffer, bytes_read, main, fd_max);
	}
	else {
		broadcast(inc_sock, server_sock, buffer, bytes_read, main, fd_max);
	}
	

	return;
}

int main(int argc, char *argv[]) {


	
	Flags flags = default_flags;
	char node_arr[IPV6_ADDRSTRLEN + 1] = "\0";
	char service[MAX_PORT_NUM_CHAR_LEN + 1]; // todo: test overflow?
	int res = process_args(argc, argv, node_arr, IPV6_ADDRSTRLEN, service, MAX_PORT_NUM_CHAR_LEN, &flags);
	if (res == -1) {
		printf("Could not process args");
		exit(EXIT_FAILURE);
	}
	char* node = strlen(node_arr) == 0 ? NULL : node_arr;

	ServerInfo si;
	si.ip = node;
	si.port = service;
	si.type = flags.server_type;

	//

	res = socket_init();
	if (res != 0) {
		perror("server: socket initialisation failed, exiting");
		exit(EXIT_FAILURE);
	}


	fd_set main_fds;
	fd_set read_fds; // temps 
	SOCKET fd_max; // biggest fd

	
	FD_ZERO(&main_fds);
	FD_ZERO(&read_fds);

	SOCKET server_sock = init_listen_socket(node, service);
	
	FD_SET(server_sock, &main_fds);
	fd_max = server_sock;

	while (true) {
		read_fds = main_fds; // copy

		int res = socket_select_read_only(fd_max+1, &read_fds, SELECT_NO_TIMEOUT);
		if (res == -1) {
			perror("Couldn't select");
			exit(EXIT_FAILURE);
		}

		for (int i = 0; i <= fd_max; i++) {
			if (!FD_ISSET(i, &read_fds)) continue;

			if (i == server_sock) {
				process_incoming_connection(i, &main_fds, &fd_max);
			}
			else {
				process_incoming_data(i, server_sock, &main_fds, fd_max, &flags);
			}
			
		}
	}

	socket_close(server_sock);
	
	printf("server: closed socket\n");

	socket_quit();

	return 0;
}
