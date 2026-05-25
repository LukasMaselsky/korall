#include "korall_internal.h"
#include "sockets.h"
#include "http_internal.h"
#include "arena.h"
#include "lookup_tables.h"
#include "cJSON.h"
#include "websocket_internal.h"
#include "array.h"
#include "config.h"

// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr


/*
	Finds which route the request is targeting and returns it
*/
static HTTPRoute* http_route_select(HTTPRequest *req, const HTTPRoutes *routes) {
	if (routes == NULL) return NULL;
	const char *path = req->start_line->request_target;
	char sub_path[MAX_HTTP_URL_LEN + 1] = { 0 };
	if (fill_string_char(&path, sub_path, MAX_HTTP_URL_LEN, '?') == 0) {
		path = sub_path;
	}
	const HTTPMethod method = req->start_line->method;

	for (HTTPRoute* route = routes->routes; route != routes->routes + routes->route_count; route++) {
		if (route->method != method) continue;

		if (strcmp(route->path, path) != 0) continue;

		return route;
	}
	return NULL;
}

static WebsocketRoute* ws_route_select(HTTPRequest* req, const WebsocketRoutes* routes) {
	if (routes == NULL) return NULL;
	const char* path = req->start_line->request_target;
	char sub_path[MAX_HTTP_URL_LEN + 1] = { 0 };
	if (fill_string_char(&path, sub_path, MAX_HTTP_URL_LEN, '?') == 0) {
		path = sub_path;
	}

	for (WebsocketRoute* route = routes->routes; route != routes->routes + routes->route_count; route++) {

		if (strcmp(route->path, path) != 0) continue;

		return route;
	}
	return NULL;
}

/**
 * @brief finds if socket belongs to a websocket connection
 * @param socket 
 * @param arr 
 * @return 
 */
static int ws_connection_select(const SOCKET socket, Array* arr) {
	if (arr == NULL) return -1;
	for (size_t i = 0; i < arr->size; i++) {
		WebsocketConnection* wsi = (WebsocketConnection*)array_get(arr, i);
		if (wsi->socket == socket) return i;
	}
	return -1;
}

static bool http_domain_port_match_server(const HTTPRequest* req) {

	if (req->start_line->method == HTTP_CONNECT) {
		// know rt is valid domain:port
		char domain[MAX_DOMAIN_LEN + 1] = { 0 };
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = { 0 };
		const char* str = req->start_line->request_target;

		if (fill_string_char(&str, domain, MAX_DOMAIN_LEN, ':') == -1) return false;
		if (fill_string_char(&str, port, MAX_PORT_NUM_CHAR_LEN, '\0') == -1) return false;

		if (strcmp(g_config.domain.chars, domain) != 0 ||
			strcmp(g_config.port.chars, port) != 0) return false;
	}
	
	if (strcmp(g_config.domain.chars, req->host->domain) != 0 ||
		strcmp(g_config.port.chars, req->host->port) != 0) return false;
	

	return true;
}

static void websocket_process_data(
	const SOCKET inc_sock,
	const char* data,
	fd_set* main,
	Array* ws_arr,
	int wsc_index
) {
	// process frame

	Arena inc_arena = arena_init(WS_ARENA_SIZE);
	Arena out_arena = arena_init(WS_ARENA_SIZE);

	WebsocketFrame* in_wsf;
	in_wsf = (WebsocketFrame *)arena_alloc(&inc_arena, sizeof(*in_wsf));

	uint8_t* in_payload;
	in_payload = (uint8_t*)arena_alloc(&inc_arena, WS_FRAME_PAYLOAD_SIZE);
	in_wsf->data = in_payload;

	WebsocketFrame* out_wsf;
	out_wsf = (WebsocketFrame *)arena_alloc(&out_arena, sizeof(*out_wsf));


	if (websocket_frame_decode((uint8_t*)data, in_wsf) == -1) {
		printf("server: invalid websocket message received, syntax\n");
		goto websocket_process_data_end;
	}
	in_wsf->socket = inc_sock;

	if (!(in_wsf->finished)) goto websocket_process_data_end; // todo: add continuous support

	if (in_wsf->opcode == WS_OP_PONG) goto websocket_process_data_end; // ignore pong

	// send pong when you get ping
	if (in_wsf->opcode == WS_OP_PING) {
		if (websocket_frame_construct_pong(out_wsf, inc_sock, false, 0) == -1) {
			printf("server: failed to construct pong message\n");
			goto websocket_process_data_end;
		}

		if (korall_ws_frame_send(out_wsf) == -1) {
			printf("server: failed to send pong message\n");
		}
		goto websocket_process_data_end;
	}

	// if close frame, remove from both sets and close socket connection

	if (in_wsf->opcode == WS_OP_CLOSE) {

		// todo: close codes
		if (websocket_frame_construct_close(out_wsf, inc_sock, WS_CC_1000, false, 0) == -1) {
			printf("server: failed to construct close message\n");
			goto websocket_process_data_end;
		}

		if (korall_ws_frame_send(out_wsf) == -1) {
			printf("server: failed to send close message\n");
			goto websocket_process_data_end;
		}

		if (socket_close(inc_sock) == -1) {
			printf("server: failed to close socket ");
			socket_print(inc_sock);
			printf("\n");
		};

		// remove from ws connections array
		array_remove(ws_arr, wsc_index);
		FD_CLR(inc_sock, main);

		goto websocket_process_data_end;
	}

	// CALLBACK

	WebsocketConnection* wsc = array_get(ws_arr, wsc_index);
	WebsocketRoute* route = wsc->route;

	route->callback(in_wsf);

websocket_process_data_end:
	arena_free(&inc_arena);
	arena_free(&out_arena);
	return;
}

/**
 * @brief send route not found response
 * @return 
 */
static void route_not_found(HTTPResponse* res, SOCKET inc_sock) {
	int err = http_response_construct(res, HTTP_SC_404, g_config.name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Route not found"));
	if (err == -1) return;
	if (http_response_send(inc_sock, res) == -1) {
		printf("Failed to send responses\n");
	};
	return;
}

static bool req_is_ws_upgrade(const HTTPRequest* req) {
	return req->ws->has_key && req->ws->has_connection && req->ws->has_upgrade && req->ws->has_version;
}

static void http_process_request(
	const SOCKET inc_sock,
	const char* data,
	Array* ws_arr,
	const HTTPRoutes *routes,
	const WebsocketRoutes *ws_routes
) {

	Arena req_arena = arena_init(HTTP_REQ_ARENA_SIZE);
	Arena res_arena = arena_init(HTTP_RES_ARENA_SIZE);

	HTTPRequest *req = http_request_init(&req_arena);
	HTTPResponse *res = http_response_init(&res_arena);

	// first validate format
	HTTPError parse_res = http_request_parse(data, req);
	if (parse_res != HTTP_SUCCESS) {
		printf("server: invalid HTTP request received, syntax\n");

		HTTPStatusCode sc;
		HTTPMediaType mt;
		const char* message = http_error_response_info(parse_res, &sc, &mt);

		int err = http_response_construct(res, sc, g_config.name.chars, mt, message);
		if (err == -1) goto http_process_request_end;
		if (http_response_send(inc_sock, res) == -1) {
			printf("Failed to send responses\n");
		}
		goto http_process_request_end;
	}

	// check if Host matches server domain + port, also if CONNECT req, if rt matches it aswell
	if (!http_domain_port_match_server(req)) { 
		printf("server: invalid HTTP request received, host\n");
		int err = http_response_construct(res, HTTP_SC_400, g_config.name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Invalid Host header."));
		if (err == -1) goto http_process_request_end;
		if (http_response_send(inc_sock, res) == -1) {
			printf("Failed to send responses\n");
		}
		goto http_process_request_end;
	}

	printf("server: valid HTTP request received\n");

	
	if (req_is_ws_upgrade(req)) {

		// check if WS route exists

		WebsocketRoute* wsr = ws_route_select(req, ws_routes);
		if (wsr == NULL) {
			route_not_found(res, inc_sock);
			goto http_process_request_end;
		}

		// send 101
		int err = http_response_ws_construct(res, req->ws->accept, g_config.name.chars);
		if (err == -1) goto http_process_request_end;
		if (http_response_send(inc_sock, res) == -1) {
			printf("Failed to send responses\n");
		}
		else {
			WebsocketConnection wsc = { .route = wsr, .socket = inc_sock };
			array_push(ws_arr, &wsc);
		}

		goto http_process_request_end;
	}

	printf("server: sending HTTP response\n\n");

	const HTTPRoute* route = http_route_select(req, routes);
	if (route == NULL) {
		// send 404 if no matching route
		route_not_found(res, inc_sock);
		goto http_process_request_end;
	}
	route->callback(req, res); // CALL CALLBACK

	if (res->start_line.chars[0] == '\0') {
		printf("Failed to send response, no start line set\n");
		goto http_process_request_end;
	}
	
	if (http_response_send(inc_sock, res) == -1) {
		printf("Failed to send responses\n");
	}

http_process_request_end:
	http_response_free(&res_arena);
	http_request_free(&req_arena);
	return;
}

/**
 * @brief Initialise a socket for listening
 * @return open socket
 */
static SOCKET init_listen_socket() {
	int res;
	SOCKET sock;
	struct addrinfo *serverinfo, *addrinfo;

	const char* node = g_config.domain.chars;
	const char* service = g_config.port.chars;
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
	printf("server: started \"%s\"\n", g_config.name.chars);
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
	fd_set* main, 
	Array* ws_arr, 
	const HTTPRoutes *http_routes,
	const WebsocketRoutes *ws_routes
) {
	// todo: change to heap for larger buffer
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

	int wsci;
	if ((wsci = ws_connection_select(inc_sock, ws_arr)) != -1) {
		websocket_process_data(inc_sock, buffer, main, ws_arr, wsci);
	}
	else {
		http_process_request(inc_sock, buffer, ws_arr, http_routes, ws_routes);
	}


	// TODO
	//if (config->type == ST_HTTP) {
	//}
	//else {
		//broadcast(inc_sock, server_sock, buffer, bytes_read, main, fd_max);
	//}
	

	return;
}

// PUBLIC FUNCTIONS

static void http_routes_free(HTTPRoutes* routes) {
	if (routes != NULL) {
		free(routes->routes);
		free(routes);
	}
}

HTTPRoutes* korall_http_routes_init() {
	size_t capacity = sizeof(HTTPRoute) * HTTP_ROUTES_CAPACITY;
	HTTPRoutes* routes = (HTTPRoutes*)safe_calloc(1, sizeof(HTTPRoutes));
	routes->routes = (HTTPRoute*)safe_calloc(capacity, sizeof(HTTPRoute));
	routes->capacity = capacity;
	routes->route_count = 0;
	return routes;
}

void korall_http_routes_add(HTTPRoutes* routes, const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*)) {
	if (path == NULL) {
		printf("Failed to add route, path cannot be NULL.\n");
		return;
	}
	if (callback == NULL) {
		printf("Failed to add route, callback cannot be NULL.\n");
		return;
	}
	if (lookup_int_str(method, &http_method_lookup_table) == NULL) {
		printf("Failed to add route, method is not valid.\n");
		return;
	}
	size_t count = routes->route_count;
	if (count >= routes->capacity) {
		printf("Failed to add route, maximum route count exceeded.\n");
		return;
	}
	HTTPRoute route = { .path = path, .method = method, .callback = callback };
	memcpy(routes->routes + count, &route, sizeof(route));
	routes->route_count = count + 1;
}

static void ws_routes_free(WebsocketRoutes* routes) {
	if (routes != NULL) {
		free(routes->routes);
		free(routes);
	}
}

WebsocketRoutes* korall_ws_routes_init() {
	size_t capacity = sizeof(WebsocketRoute) * WS_ROUTES_CAPACITY;
	WebsocketRoutes* routes = (WebsocketRoutes*)safe_calloc(1, sizeof(WebsocketRoutes));
	routes->routes = (WebsocketRoute*)safe_calloc(capacity, sizeof(WebsocketRoute));
	routes->capacity = capacity;
	routes->route_count = 0;
	return routes;
}

void korall_ws_routes_add(WebsocketRoutes* routes, const char* path, void (* const callback)(const WebsocketFrame*)) {
	if (path == NULL) {
		printf("Failed to add route, path cannot be NULL.\n");
		return;
	}
	if (callback == NULL) {
		printf("Failed to add route, callback cannot be NULL.\n");
		return;
	}
	
	size_t count = routes->route_count;
	if (count >= routes->capacity) {
		printf("Failed to add route, maximum route count exceeded.\n");
		return;
	}
	WebsocketRoute route = { .path = path, .callback = callback };
	memcpy(routes->routes + count, &route, sizeof(route));
	routes->route_count = count + 1;
}

void korall_run(const char *config_path, const HTTPRoutes* http_routes, const WebsocketRoutes *ws_routes) {

	// config

	ServerConfig* config = config_init(config_path);
	
	// sockets

	int res = socket_init();
	if (res != 0) {
		perror("server: socket initialisation failed, exiting");
		exit(EXIT_FAILURE);
	}


	fd_set main_fds = { 0 };
	fd_set read_fds = { 0 }; // temps
	SOCKET fd_max; // biggest fd
	
	WebsocketConnection ws_arr_data[FD_SETSIZE] = { 0 };
	Array ws_arr = { 0 };
	array_create_stack(&ws_arr, ws_arr_data, sizeof(WebsocketConnection), FD_SETSIZE);

	FD_ZERO(&main_fds);
	FD_ZERO(&read_fds);

	SOCKET server_sock = init_listen_socket();
	
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
				process_incoming_data(i, &main_fds, &ws_arr, http_routes, ws_routes);
			}

		}
	}

	socket_close(server_sock);

	printf("server: closed socket\n");

	socket_quit();

	// free config and routes

	http_routes_free(http_routes);
	ws_routes_free(ws_routes);
	config_free(config);

	return;
}
