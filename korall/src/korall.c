#include "korall_internal.h"
#include "sockets.h"
#include "http_internal.h"
#include "arena.h"
#include "lookup_tables.h"
#include "cJSON.h"

// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr

const ServerConfig default_config = {
	.domain = DEFAULT_DOMAIN,
	.port = DEFAULT_PORT,
	.name = DEFAULT_SERVER_NAME,
};

/*
	Finds which route the request is targeting and returns it
*/
static Route* http_route_select(HTTPRequest *req, const Routes *routes) {
	const char *path = req->start_line->request_target;
	const HTTPMethod method = req->start_line->method;

	for (Route* route = routes->routes; route != routes->routes + routes->route_count; route++) {
		if (route->method != method) continue;

		if (strcmp(route->path, path) != 0) continue;

		return route;
	}
	return NULL;
}

static bool http_domain_port_match_server(ServerConfig* config, const HTTPRequest* req) {

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
		if (strcmp(config->domain.chars, domain) != 0 ||
			strcmp(config->port.chars, port) != 0) return false;
	}
	
	if (strcmp(config->domain.chars, req->headers->host->domain) != 0 ||
		strcmp(config->port.chars, req->headers->host->port) != 0) return false;
	

	return true;
}

static void http_process_request(
	const SOCKET inc_sock, 
	const SOCKET server_sock, 
	const char* data,
	const fd_set* main, 
	const SOCKET fd_max,
	const ServerConfig *config,
	const Routes *routes
) {

	Arena req_arena = arena_init(HTTP_REQ_SIZE);
	Arena res_arena = arena_init(HTTP_RES_ARENA_SIZE);

	HTTPRequest *req = http_request_init(&req_arena);
	HTTPResponse *res = http_response_init(&res_arena);

	// first validate format
	HTTPError parse_res = http_parse_request(data, req);
	if (parse_res != HTTP_SUCCESS) {
		printf("server: invalid HTTP request received, syntax\n");

		HTTPStatusCode sc;
		HTTPMediaType mt;
		const char* message = http_error_response_info(parse_res, &sc, &mt);

		int err = http_response_construct(res, sc, config->name.chars, mt, message);
		if (err == NULL) return;
		if (http_response_send(inc_sock, server_sock, res, main) == -1) return;
		http_response_free(&res_arena, res);
		http_request_free(&req_arena, req);
		return;
	}

	// check if Host matches server domain + port, also if OPTIONS req, if rt matches it aswell
	if (!http_domain_port_match_server(config, req)) { 
		printf("server: invalid HTTP request received, host\n");
		int err = http_response_construct(res, HTTP_SC_400, config->name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Invalid Host header."));
		if (err == -1) return;
		if (http_response_send(inc_sock, server_sock, res, main) == -1) {
			printf("Failed to send responses\n");
		}
		http_response_free(&res_arena, res);
		http_request_free(&req_arena, req);
		return;
	}

	printf("server: valid HTTP request received\n");
	printf("server: sending HTTP response\n\n");

	if (routes == NULL) return; // no route handlers

	const Route* route = http_route_select(req, routes);
	if (route == NULL) {
		// send 404 if no matching route
		int err = http_response_construct(res, HTTP_SC_404, config->name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Route not found"));
		if (err == -1) return;
		if (http_response_send(inc_sock, server_sock, res, main) == -1) {
			printf("Failed to send responses\n");
		};
		
		http_response_free(&res_arena, res);
		http_request_free(&req_arena, req);
		return;
	}
	route->callback(req, res); // CALL CALLBACK

	if (http_response_send(inc_sock, server_sock, res, main) == -1) {
		printf("Failed to send responses\n");
	}

	http_response_free(&res_arena, res);
	http_request_free(&req_arena, req);

	return;
}

/*
	Initialise a socket for listening
*/
static SOCKET init_listen_socket(ServerConfig *config) {
	int res;
	SOCKET sock;
	struct addrinfo *serverinfo, *addrinfo;

	const char* node = config->domain.chars;
	const char* service = config->port.chars;
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
	printf("server: started \"%s\"\n", config->name.chars);
	printf("server: opened socket on %s PORT %s (%s)\n", ip, service, ipver);

	res = socket_listen(sock);
	if (res == -1) {
		perror("server: socket listen\n");
		exit(EXIT_FAILURE);
	}

	return sock;
}

static void process_incoming_connection(SOCKET sock, fd_set* main, SOCKET* fd_max) {
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

static void broadcast(SOCKET inc_sock, SOCKET server_sock, const char* data, int data_len, fd_set* main, SOCKET fd_max) {
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

static void process_incoming_data(
	const SOCKET inc_sock, 
	const SOCKET server_sock, 
	const fd_set* main, 
	const SOCKET fd_max, 
	const ServerConfig *config, 
	const Routes *routes
) {
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

	http_process_request(inc_sock, server_sock, buffer, main, fd_max, config, routes);
	// TODO
	//if (config->type == ST_HTTP) {
	//}
	//else {
		//broadcast(inc_sock, server_sock, buffer, bytes_read, main, fd_max);
	//}
	

	return;
}

// PUBLIC FUNCTIONS

HTTPConfigError http_config_init(const char *path, ServerConfig *config) {

	const char* config_file_name = SERVER_CONFIG_FILE_NAME;
	char file_path[MAX_FILE_PATH + 1] = { 0 };

	if (path == NULL) {
		strcpy(file_path, config_file_name);
	}
	else {
		size_t path_len = strlen(path);
		if (path_len > MAX_FILE_PATH) { 
			printf("File path too long.");
			return HTTP_CONF_ERROR; 
		};
		strcpy(file_path, path);
		if (strlen(config_file_name) + path_len > MAX_FILE_PATH) {
			printf("File path too long.");
			return HTTP_CONF_ERROR;
		};
		strcat(file_path, config_file_name);
	}
	
	FILE* fp = fopen(file_path, "r");
	if (fp == NULL) { 
		printf("Could not find a korall_config.json, file using default config.\nIf you are using a custom config, make sure the path is correct.");
		return HTTP_CONF_DEFAULT;
	};

	// read the file contents into a string
	char buffer[HTTP_CONFIG_BUFFER_LEN + 1];
	size_t len = fread(buffer, 1, sizeof(buffer), fp);
	fclose(fp);

	// parse the JSON data
	cJSON* json = cJSON_Parse(buffer);
	if (json == NULL) {
		const char* error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			printf("%s\n", error_ptr);
		}
		cJSON_Delete(json);
		return HTTP_CONF_ERROR;
	}

	// access the JSON data

	// go through all ServerConfig

	// server name

	cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
	if (!(cJSON_IsString(name) && (name->valuestring != NULL))) {
		printf("Config \"name\" field is not valid.\n");
		return HTTP_CONF_ERROR;
	}
	strncpy(config->name.chars, name->valuestring, config->name.size);
	
	// domain

	cJSON* domain = cJSON_GetObjectItemCaseSensitive(json, "domain");
	if (!(cJSON_IsString(domain) && (domain->valuestring != NULL))) {
		printf("Config \"domain\" field is not valid.\n");
		return HTTP_CONF_ERROR;
	}
	strncpy(config->domain.chars, domain->valuestring, config->domain.size);

	// port

	cJSON* port = cJSON_GetObjectItemCaseSensitive(json, "port");
	if (!(cJSON_IsNumber(port))) {
		printf("Config \"port\" field is not valid.\n");
		return HTTP_CONF_ERROR;
	}
	if (!is_valid_port_num(port->valueint)) {
		printf("Config \"port\" field number is not valid, must be between %d and %d.\n", MIN_PORT_NUM, MAX_PORT_NUM);
		return HTTP_CONF_ERROR;
	};

	char* port_str[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	int_to_str(port->valueint, port_str);
	strncpy(config->port.chars, port_str, config->port.size);

	//

	cJSON_Delete(json);
	return HTTP_CONF_SUCCESS;
}

static void http_routes_free(Routes* routes) {
	free(routes->routes);
	free(routes);
}

Routes* korall_routes_init() {
	size_t capacity = sizeof(Route) * HTTP_ROUTES_CAPACITY;
	Routes* routes = (Routes*)safe_calloc(1, sizeof(Routes));
	routes->routes = (Route*)safe_calloc(capacity, sizeof(Route));
	routes->capacity = capacity;
	routes->route_count = 0;
	return routes;
}

void korall_routes_add(Routes* routes, const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*)) {
	if (path == NULL) {
		printf("Failed to add route, path cannot be NULL.\n");
		return;
	}
	if (callback == NULL) {
		printf("Failed to add route, callback cannot be NULL.\n");
		return;
	}
	if (lookup_int_str(method, &http_method_lookup_table) == -1) {
		printf("Failed to add route, method is not valid.\n");
		return;
	}
	size_t count = routes->route_count;
	if (count >= routes->capacity) {
		printf("Failed to add route, maximum route count exceeded.\n");
		return;
	}
	Route route = { .path = path, .method = method, .callback = callback };
	memcpy(routes->routes + count, &route, sizeof(route));
	routes->route_count = count + 1;
}

void korall_run(const char *config_path, const Routes* routes) {

	// config

	char* domain[MAX_DOMAIN_LEN + 1] = { 0 };
	char* port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
	char* name[MAX_SERVER_NAME_LEN + 1] = { 0 };

	ServerConfig config = {
		.domain = {.chars = domain, .size = MAX_DOMAIN_LEN },
		.port = {.chars = port, .size = MAX_PORT_NUM_CHAR_LEN },
		.name = {.chars = name, .size = MAX_SERVER_NAME_LEN },
	};
	HTTPConfigError conf_res = http_config_init(config_path, &config);
	
	switch (conf_res) {
		case HTTP_CONF_SUCCESS:
			break;
		case HTTP_CONF_DEFAULT:
			config = default_config;
			break;
		case HTTP_CONF_ERROR:
		default:
			return;
	}
	
	// sockets

	int res = socket_init();
	if (res != 0) {
		perror("server: socket initialisation failed, exiting");
		exit(EXIT_FAILURE);
	}


	fd_set main_fds = { 0 };
	fd_set read_fds = { 0 }; // temps 
	SOCKET fd_max; // biggest fd


	FD_ZERO(&main_fds);
	FD_ZERO(&read_fds);

	SOCKET server_sock = init_listen_socket(&config);

	FD_SET(server_sock, &main_fds);
	fd_max = server_sock;

	while (true) {
		read_fds = main_fds; // copy

		int res = socket_select_read_only(fd_max + 1, &read_fds, SELECT_NO_TIMEOUT);
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
				process_incoming_data(i, server_sock, &main_fds, fd_max, &config, routes);
			}

		}
	}

	socket_close(server_sock);

	printf("server: closed socket\n");

	socket_quit();

	// free config and routes

	http_routes_free(routes);

	return;
}
