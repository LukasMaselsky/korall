#include "server/main.h"
#include "utils.h"
#include "sockets.h"
#include "http_.h"
#include "arena.h"
#include "lookup_tables.h"

Flags default_flags = {
	.server_type = ST_HTTP,
};

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

	int val = lookup_str_int(key, flag_lookup_table, FLAG_LOOKUP_TABLE_COUNT, true);
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

static bool http_domain_port_match_server(ServerInfo* si, HTTPRequest* req) {

	if (req->start_line->method == HTTP_CONNECT) {
		// know rt is valid domain:port
		char domain[MAX_DOMAIN_LEN + 1] = { 0 };
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		const char* str = req->start_line->request_target;
		int i = 0;
		for (char c = *str; c != ':'; c = *(++str)) {
			domain[i] = c;
			i++;
		}
		str++;
		i = 0;
		for (char c = *str; c != '\0'; c = *(++str)) {
			port[i] = c;
			i++;
		}
		if (strcmp(si->domain, domain) != 0 ||
			strcmp(si->port, port) != 0) return false;
	}
	
	if (strcmp(si->domain, req->headers->host->domain) != 0 ||
		strcmp(si->port, req->headers->host->port) != 0) return false;
	

	return true;
}

static void http_process_request(
	SOCKET inc_sock, 
	SOCKET server_sock, 
	const char* data, 
	int data_len, 
	fd_set* main, 
	SOCKET fd_max,
	ServerInfo *si
) {

	Arena req_arena = arena_init(HTTP_REQ_SIZE);
	Arena res_arena = arena_init(HTTP_RES_SIZE);
	HTTPRequest *req = http_request_init(&req_arena);

	// first validate format
	HTTPError parse_res = http_parse_request(data, data_len, req);
	if (parse_res != HTTP_SUCCESS) {
		printf("server: invalid HTTP request received, syntax\n");

		HTTPStatusCode sc;
		HTTPMediaType mt;
		const char* message = http_error_response_info(parse_res, &sc, &mt);

		HTTPResponse* res = http_response_construct(&res_arena, sc, SERVER_NAME, mt, message);
		if (res == NULL) return;
		if (http_response_send(inc_sock, server_sock, res, main) == -1) return;
		http_response_free(&res_arena, res);
		http_request_free(&req_arena, req);
		return;
	}

	// check if Host matches server domain + port, also if OPTIONS req, if rt matches it aswell
	if (!http_domain_port_match_server(si, req)) { 
		printf("server: invalid HTTP request received, host\n");
		HTTPResponse* res = http_response_construct(&res_arena, HTTP_SC_400, SERVER_NAME, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Invalid Host header."));
		if (res == NULL) return;
		if (http_response_send(inc_sock, server_sock, res, main) == -1) return;
		http_response_free(&res_arena, res);
		http_request_free(&req_arena, req);
		return;
	}

	printf("server: valid HTTP request received\n");
	printf("server: sending HTTP response\n\n");

	HTTPResponse* res = http_response_construct(&res_arena, HTTP_SC_200, SERVER_NAME, HTTP_MT_TXT_PLAIN, "Hello World!");
	if (res == NULL) return;
	if (http_response_send(inc_sock, server_sock, res, main) == -1) return;
	http_response_free(&res_arena, res);


	http_request_free(&req_arena, req);

	return;
}

/*
	Initialise a socket for listening
*/
SOCKET init_listen_socket(const char* node, const char* service, ServerInfo *si) {
	int res;
	SOCKET sock;
	struct addrinfo *serverinfo, *addrinfo;


	if (node == NULL || strcmp(node, "localhost") == 0) {
		node = LOCALHOST_NODE; // default server to localhost
		si->domain = "localhost";
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

void process_incoming_data(SOCKET inc_sock, SOCKET server_sock, fd_set* main, SOCKET fd_max, Flags* flags, ServerInfo *si) {
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
		http_process_request(inc_sock, server_sock, buffer, bytes_read, main, fd_max, si);
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
	si.domain = node;
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

	SOCKET server_sock = init_listen_socket(node, service, &si);
	
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
				process_incoming_data(i, server_sock, &main_fds, fd_max, &flags, &si);
			}
			
		}
	}

	socket_close(server_sock);
	
	printf("server: closed socket\n");

	socket_quit();

	return 0;
}
