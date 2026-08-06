#include "server_http.h"
#include "socket/socket.h"
#include "http/http_internal.h"
#include "arena/arena.h"
#include "lookup/lookup_tables.h"
#include "cJSON/cJSON.h"
#include "http/websocket/websocket_internal.h"
#include "array/array.h"
#include "config/config.h"
#include "gui/gui.h"
#include "thread/thread.h"

/*
	Finds which route the request is targeting and returns it
*/
HTTPRoute *http_route_select(HTTPRequest *req, const Array *routes)
{
	if (routes == NULL)
		return NULL;
	const char *path = req->start_line->request_target;
	char sub_path[MAX_HTTP_URL_LEN + 1] = {0};
	if (fill_string_char(&path, sub_path, MAX_HTTP_URL_LEN, '?') == 0)
	{
		path = sub_path;
	}
	const HTTPMethod method = req->start_line->method;

	int i;
	array_for_loop(i, routes)
	{
		const HTTPRoute *route = (const HTTPRoute *)array_get(routes, i);
		if (route->method != method)
			continue;

		if (strcmp(route->path, path) != 0)
			continue;

		return route;
	}

	return NULL;
}

WebsocketRoute *ws_route_select(HTTPRequest *req, const Array *routes)
{
	if (routes == NULL)
		return NULL;
	const char *path = req->start_line->request_target;
	char sub_path[MAX_HTTP_URL_LEN + 1] = {0};
	if (fill_string_char(&path, sub_path, MAX_HTTP_URL_LEN, '?') == 0)
	{
		path = sub_path;
	}

	int i;
	array_for_loop(i, routes)
	{
		const WebsocketRoute *route = (const WebsocketRoute *)array_get(routes, i);
		if (strcmp(route->path, path) != 0)
			continue;

		return route;
	}

	return NULL;
}

bool http_domain_port_match_server(const HTTPRequest *req)
{
	ServerConfig *g_config = config_get();
	if (req->start_line->method == HTTP_CONNECT)
	{
		// know rt is valid domain:port
		char domain[MAX_DOMAIN_LEN + 1] = {0};
		char port[MAX_PORT_NUM_CHAR_LEN + 1] = {0};
		const char *str = req->start_line->request_target;

		if (fill_string_char(&str, domain, MAX_DOMAIN_LEN, ':') == -1)
			return false;
		if (fill_string_char(&str, port, MAX_PORT_NUM_CHAR_LEN, '\0') == -1)
			return false;

		if (strcmp(g_config->domain.chars, domain) != 0 ||
			strcmp(g_config->port.chars, port) != 0)
			return false;
	}

	if (strcmp(g_config->domain.chars, req->host->domain) != 0 ||
		strcmp(g_config->port.chars, req->host->port) != 0)
		return false;

	return true;
}

static int websocket_process_data_inner(
	WebsocketFrame *in_wsf,
	WebsocketFrame *out_wsf,
	const SOCKET inc_sock,
	const char *data,
	const WebsocketRoute *route,
	const SSL *ssl,
	bool *close)
{
	if (websocket_frame_decode((uint8_t *)data, in_wsf) == -1)
	{
		KORALL_LOG(LOG_ERR, "invalid websocket message received, syntax\n");
		return -1;
	}
	in_wsf->socket = inc_sock;
	in_wsf->ssl = ssl;

	if (!(in_wsf->finished))
		return 0; // todo: add continuous support

	if (in_wsf->opcode == WS_OP_PONG)
		return 0; // ignore pong

	// send pong when you get ping
	if (in_wsf->opcode == WS_OP_PING)
	{
		if (websocket_frame_construct_pong(out_wsf, inc_sock, false, 0) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to construct pong message\n");
			return -1;
		}

		if (korall_ws_frame_send(out_wsf) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to send pong message\n");
			return -1;
		}
		return 0;
	}

	// if close frame, remove from both sets and close socket connection

	if (in_wsf->opcode == WS_OP_CLOSE)
	{

		// todo: close codes
		if (websocket_frame_construct_close(out_wsf, inc_sock, WS_CC_1000, false, 0) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to construct close message\n");
			return -1;
		}

		if (korall_ws_frame_send(out_wsf) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to send close message\n");
			return -1;
		}

		if (socket_close(inc_sock) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to close socket ");
			socket_print(inc_sock);
			printf("\n");
		};

		*close = true;

		return 0;
	}

	// CALLBACK

	route->callback(in_wsf);

	return 0;
}

/**
 * @brief
 * @param inc_sock
 * @param data
 * @param route
 * @return true if close
 */
bool websocket_process_data(
	const SOCKET inc_sock,
	const char *data,
	const WebsocketRoute *route,
	const SSL *ssl)
{
	bool close = false;

	Arena inc_arena = arena_init(WS_ARENA_SIZE);
	Arena out_arena = arena_init(WS_ARENA_SIZE);

	WebsocketFrame *in_wsf;
	in_wsf = (WebsocketFrame *)arena_alloc(&inc_arena, sizeof(*in_wsf));

	uint8_t *in_payload;
	in_payload = (uint8_t *)arena_alloc(&inc_arena, WS_FRAME_PAYLOAD_SIZE);
	in_wsf->data = in_payload;

	WebsocketFrame *out_wsf;
	out_wsf = (WebsocketFrame *)arena_alloc(&out_arena, sizeof(*out_wsf));

	websocket_process_data_inner(
		in_wsf,
		out_wsf,
		inc_sock,
		data,
		route,
		ssl,
		&close);

	arena_free(&inc_arena);
	arena_free(&out_arena);
	return close;
}

/**
 * @brief send route not found response
 * @return
 */
void route_not_found(HTTPResponse *res, SOCKET inc_sock)
{
	KORALL_LOG(LOG_ERR, "route not found\n");
	ServerConfig *g_config = config_get();
	int err = http_response_construct(res, HTTP_SC_404, g_config->name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Route not found"));
	if (err == -1)
		return;
	if (http_response_send(inc_sock, res) == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to send response\n");
	}
	return;
}

static bool req_is_ws_upgrade(const HTTPRequest *req)
{
	return req->ws->has_key && req->ws->has_connection && req->ws->has_upgrade && req->ws->has_version;
}

static int ws_upgrade(
	const HTTPRequest *req,
	HTTPResponse *res,
	const ServerConfig *config,
	const SOCKET inc_sock,
	const Array *ws_routes,
	bool *is_websocket,
	WebsocketRoute **websocket_route)
{
	// check if WS route exists

	WebsocketRoute *wsr = ws_route_select(req, ws_routes);
	if (wsr == NULL)
	{
		route_not_found(res, inc_sock);
		return -1;
	}

	// send 101
	int err = http_response_ws_construct(res, req->ws->accept, config->name.chars);
	if (err == -1)
		return -1;
	if (http_response_send(inc_sock, res) == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to send response\n");
	}
	else
	{
		*is_websocket = true;
		*websocket_route = wsr;
	}
}

static int http_process_request_options(
	const HTTPRequest *req,
	HTTPResponse *res,
	const ServerConfig *config,
	const SOCKET inc_sock)
{
	KORALL_LOG(LOG_INFO, "OPTIONS request received\n");

	// Access-Control-Request-Method
	char acrm[MAX_HTTP_METHOD_STR_LEN + 1] = {0};
	int acrm_res = korall_request_header_get(req, "Access-Control-Request-Method", acrm, MAX_HTTP_METHOD_STR_LEN);
	if (acrm_res == -1)
	{
		KORALL_LOG(LOG_ERR, "invalid HTTP request received, host\n");
		int err = http_response_construct(res, HTTP_SC_400, config->name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "OPTIONS request must contain Access-Control-Request-Method header."));
		if (err == -1)
			return -1;
		if (http_response_send(inc_sock, res) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to send response\n");
		}
		return -1;
	}

	// set response header on which methods are allowed

	char stringified_methods[ALL_METHODS_LIST_STR_LEN + 1] = {0};
	int am = http_allowed_methods(config->allow_methods, stringified_methods, ALL_METHODS_LIST_STR_LEN);
	if (am == -1)
		return -1;
	korall_response_header_set(res, "Access-Control-Allow-Methods", stringified_methods);

	// Access-Control-Request-Headers
	char acrh[MAX_HTTP_HEADER_VALUE_LEN + 1] = {0};
	int acrh_res = korall_request_header_get(req, "Access-Control-Request-Headers", acrh, MAX_HTTP_HEADER_VALUE_LEN);
	if (acrh_res != -1)
	{
		char stringified_rqh[MAX_HTTP_HEADER_VALUE_LEN + 1] = {0};
		http_allowed_headers(config->allow_headers, stringified_rqh, MAX_HTTP_HEADER_VALUE_LEN);
		korall_response_header_set(res, "Access-Control-Allow-Headers", stringified_rqh);
	}
}

static int http_process_request_inner(
	const HTTPRequest *req,
	HTTPResponse *res,
	const ServerConfig *config,
	const SOCKET inc_sock,
	const char *data,
	const Array *routes,
	const Array *ws_routes,
	bool *is_websocket,
	WebsocketRoute **websocket_route,
	bool *should_close)
{
	// first validate format
	HTTPError parse_res = http_request_parse(data, req);
	if (parse_res != HTTP_SUCCESS)
	{
		KORALL_LOG(LOG_ERR, "invalid HTTP request received, syntax\n");

		HTTPStatusCode sc;
		HTTPMediaType mt;
		const char *message = http_error_response_info(parse_res, &sc, &mt);

		int err = http_response_construct(res, sc, config->name.chars, mt, message);
		if (err == -1)
			return -1;
		if (http_response_send(inc_sock, res) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to send response\n");
		}
		return -1;
	}

	// check if Host matches server domain + port, also if CONNECT req, if rt matches it aswell
	if (!http_domain_port_match_server(req))
	{
		KORALL_LOG(LOG_ERR, "invalid HTTP request received, host\n");
		int err = http_response_construct(res, HTTP_SC_400, config->name.chars, HTTP_MT_APP_JSON, ERROR_MESSAGE("Bad request", "Invalid Host header."));
		if (err == -1)
			return -1;
		if (http_response_send(inc_sock, res) == -1)
		{
			KORALL_LOG(LOG_ERR, "failed to send response\n");
		}
		return -1;
	}

	// Access-Control-Allow-Credentials

	bool creds = false;
	if (config->allow_credentials)
	{
		creds = true;
		korall_response_header_set(res, "Access-Control-Allow-Credentials", "true");
		korall_response_header_set(res, "Vary", "Origin");
	}

	// check if origin is allowed (CORS)
	char allow_origin[MAX_DOMAIN_LEN + 1] = {0};
	int vo_res = http_verify_origin(config->allow_origins, req, creds, allow_origin, MAX_DOMAIN_LEN);
	if (vo_res != -1)
	{
		korall_response_header_set(res, "Access-Control-Allow-Origin", allow_origin);
	}

	// OPTIONS request
	if (req->start_line->method == HTTP_OPTIONS)
	{
		if (http_process_request_options(req, res, config, inc_sock) == -1)
			return -1;
	}

	KORALL_LOG(LOG_INFO, "valid HTTP request received\n");

	if (req_is_ws_upgrade(req))
	{
		return ws_upgrade(req, res, config, inc_sock, ws_routes, is_websocket, websocket_route);
	}

	KORALL_LOG(LOG_INFO, "sending HTTP response\n");

	if (req->start_line->method != HTTP_OPTIONS)
	{
		const HTTPRoute *route = http_route_select(req, routes);
		if (route == NULL)
		{
			// send 404 if no matching route
			route_not_found(res, inc_sock);
			return -1;
		}
		route->callback(req, res); // CALL CALLBACK
	}
	else
	{
		// CORS preflight 204 response
		int err = http_response_construct(res, HTTP_SC_204, config->name.chars, HTTP_MT_APP_JSON, NULL);
		if (err == -1)
			return -1;
	}

	if (res->start_line.chars[0] == '\0')
	{
		KORALL_LOG(LOG_ERR, "failed to send response, no start line set\n");
		return -1;
	}

	if (http_response_send(inc_sock, res) == -1)
	{
		KORALL_LOG(LOG_ERR, "failed to send responses\n");
	}

	// check if Connection: close

	char con[20] = {0}; // todo: no num
	if (korall_request_header_get(req, "Connection", con, 20) == -1)
		return -1;

	if (strcmp_ci(con, "close") == 0)
	{
		*should_close = true;
	}
	return 0;
}

/**
 * @brief
 * @param inc_sock
 * @param data
 * @param routes
 * @param ws_routes
 * @param is_websocket
 * @param websocket_route
 * @return true if should close connection
 */
bool http_process_request(
	const SOCKET inc_sock,
	const char *data,
	const Array *routes,
	const Array *ws_routes,
	bool *is_websocket,
	WebsocketRoute **websocket_route,
	const SSL *ssl)
{
	ServerConfig *g_config = config_get();
	bool should_close = false;
	Arena req_arena = arena_init(HTTP_REQ_ARENA_SIZE);
	Arena res_arena = arena_init(HTTP_RES_ARENA_SIZE);

	HTTPRequest *req = http_request_init(&req_arena);
	HTTPResponse *res = http_response_init(&res_arena);
	res->ssl = ssl;

	http_process_request_inner(
		req,
		res,
		g_config,
		inc_sock,
		data,
		routes,
		ws_routes,
		is_websocket,
		websocket_route,
		&should_close);

	http_response_free(&res_arena);
	http_request_free(&req_arena);
	return should_close;
}

/**
 * @brief Initialise a socket for listening
 * @return open socket or INVALID_SOCKET on error
 */
SOCKET init_listen_socket()
{
	ServerConfig *g_config = config_get();
	int res;
	SOCKET sock;
	struct addrinfo *serverinfo, *addrinfo;

	const char *node = g_config->domain.chars;
	const char *service = g_config->port.chars;
	res = get_addr_info(node, service, &serverinfo);

	if (res != 0)
	{
		KORALL_LOG(LOG_ERR, "invalid \"domain\" header\n");
		return INVALID_SOCKET;
	}

	// loop through all the results and bind to the first we can
	for (addrinfo = serverinfo; addrinfo != NULL; addrinfo = addrinfo->ai_next)
	{
		sock = socket_create(addrinfo);
		if (sock == -1)
		{
			KORALL_LOG(LOG_ERR, "socket creation failed\n");
			continue;
		}

		res = socket_reuse_port(sock);
		if (res == -1)
		{
			KORALL_LOG(LOG_ERR, "setsockopt failed\n");
			return INVALID_SOCKET;
		}

		res = socket_bind(sock, addrinfo);
		if (res == -1)
		{
			socket_close(sock);
			KORALL_LOG(LOG_ERR, "socket bind failed\n");
			continue;
		}

		break;
	}

	freeaddrinfo(serverinfo);

	if (addrinfo == NULL)
	{
		KORALL_LOG(LOG_ERR, "failed to freeaddrinfo");
		return INVALID_SOCKET;
	}

	char ip[IPV6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];
	get_ip_info_addr(addrinfo, ip, sizeof(ip), ipver, sizeof(ipver));
	KORALL_LOG(LOG_INFO, "started \"%s\"\n", g_config->name.chars);
	KORALL_LOG(LOG_INFO, "opened socket on %s PORT %s (%s)\n", ip, service, ipver);

	res = socket_listen(sock);
	if (res == -1)
	{
		KORALL_LOG(LOG_ERR, "socket listen failed\n");
		return INVALID_SOCKET;
	}

	return sock;
}

/**
 * @brief
 * @param sock - server_sock
 * @param ssl_ctx
 * @param ssl_p
 * @return
 */
SOCKET process_incoming_connection(SOCKET sock, SSL_CTX *ssl_ctx, SSL **ssl_p)
{
	SOCKET incoming;
	struct sockaddr_storage incoming_addr;
	socklen_t incoming_addr_len = sizeof(incoming_addr);
	char ip[IPV6_ADDRSTRLEN];
	char ipver[IP_VER_STR_LEN];

	incoming = socket_accept(sock, &incoming_addr, &incoming_addr_len);
	if (incoming == INVALID_SOCKET)
	{
		KORALL_LOG(LOG_ERR, "failed to accept connection\n");
		return incoming;
	}

	get_ip_info_storage(&incoming_addr, ip, sizeof(ip), ipver, sizeof(ipver));
	KORALL_LOG(LOG_INFO, "got connection from %s (%s)\n", ip, ipver);

	// tls handshake

	if (ssl_ctx == NULL)
		return incoming;

	SSL *ssl = SSL_new(ssl_ctx);
	*ssl_p = ssl;
	if (ssl == NULL)
	{
		ERR_print_errors_fp(stderr);
		return incoming;
	}

	SSL_set_fd(ssl, incoming);

	if (SSL_accept(ssl) <= 0)
	{
		KORALL_LOG(LOG_ERR, "TLS handshake failed\n");
		ERR_print_errors_fp(stderr);
		SSL_shutdown(ssl);
		SSL_free(ssl);
		*ssl_p = NULL;
		// todo: close socket ?
		return incoming;
	}

	KORALL_LOG(LOG_INFO, "TLS handshake successful\n");

	return incoming;
}

/**
 * @brief thread that processes data on a socket connection
 * @param arg ProcessDataArgs
 */
void process_incoming_data(void *arg)
{

	ProcessDataArgs *p_args = (ProcessDataArgs *)arg;
	const SOCKET inc_sock = p_args->sock;
	const Array *http_routes = p_args->http_routes;
	const Array *ws_routes = p_args->ws_routes;
	const size_t thread_num = p_args->thread_num;
	pthread_mutex_t *lock = p_args->lock;
	Array *thread_arr = p_args->thread_arr;
	SSL *ssl = p_args->ssl;
	unsigned long gui_thread_id = p_args->gui_thread_id;

	// todo: change to heap for larger buffer
	char buffer[READ_BUFFER_LEN] = {0}; // buffer for client data
	size_t buffer_len = READ_BUFFER_LEN;

	bool is_websocket = false;
	WebsocketRoute *ws_route = NULL;
	bool close = false;

	while (true)
	{
		int bytes_read = socket_receive_secure(inc_sock, buffer, buffer_len - 1, 0, ssl);
		if (bytes_read <= 0)
		{
			if (bytes_read == 0)
			{
				KORALL_LOG(LOG_INFO, "socket ");
				socket_print(inc_sock);
				printf(" closed connection\n");
			}
			else
			{
				KORALL_LOG(LOG_ERR, "couldn't read from ");
				socket_print(inc_sock);
				printf("\n");
			}

			socket_close(inc_sock);
			break;
		}

		buffer[bytes_read] = '\0';
		KORALL_LOG(LOG_INFO, "received data from ");
		socket_print(inc_sock);
		printf("\n");
		KORALL_LOG(LOG_INFO, "'%s'\n", buffer);

		// todo: TEMP
		// msg_queue_post(0, gui_thread_id, buffer, bytes_read + 1);
		//

		if (is_websocket && ws_route != NULL)
		{
			close = websocket_process_data(inc_sock, buffer, ws_route, ssl);
		}
		else
		{
			close = http_process_request(inc_sock, buffer, http_routes, ws_routes, &is_websocket, &ws_route, ssl);
		}
		if (close)
			break;
		memset(buffer, 0, buffer_len);
	}

#ifndef _WIN32
	pthread_mutex_lock(lock);
	// set running state of thread to false
	ThreadState *old_ts = (ThreadState *)array_get(thread_arr, thread_num);
	ThreadState new_ts = {.running = false, .thread = old_ts->thread};
	array_set(thread_arr, thread_num, (void *)&new_ts);
#endif
	free(arg);

#ifndef _WIN32
	pthread_mutex_unlock(lock);
#endif
}

/**
 * @brief cleans up finished threads to free up array so they can be reused
 * @param threads
 * @param size
 */
void sync_threads(Array *thread_arr, pthread_mutex_t *lock)
{
	if (thread_arr == NULL || thread_arr->data == NULL)
		return;

#ifndef _WIN32
	pthread_mutex_lock(lock);
#endif
	size_t *remove_list = safe_calloc(thread_arr->size, sizeof(size_t));
	size_t len = 0;
	for (size_t i = 0; i < thread_arr->size; i++)
	{

		ThreadState *ts = (ThreadState *)array_get(thread_arr, i);

#ifdef _WIN32
		DWORD res = WaitForSingleObject(ts->thread, 0);
		if (res != WAIT_OBJECT_0)
			continue;
		CloseHandle(ts->thread);
#else
		if (ts->running)
			continue;
		pthread_join(ts->thread, NULL);
#endif
		remove_list[len] = i;
		len++;
	}
	array_remove_list(thread_arr, remove_list, len);
	free(remove_list);

#ifndef _WIN32
	pthread_mutex_unlock(lock);
#endif
}

/**
 * @brief starts thread and adds it state to array
 * @param thread_arr
 * @param t_args
 * @return
 */
void create_data_process_thread(Array *thread_arr, ProcessDataArgs *t_args)
{

	pthread_mutex_t *lock = t_args->lock;
	THREAD_T thread;

#ifndef _WIN32
	pthread_mutex_lock(lock);
#endif

	if (thread_create(&thread, process_incoming_data, (void *)t_args, NULL) != -1)
	{
		ThreadState ts = {.running = true, .thread = thread};
		KORALL_LOG(LOG_INFO, "created thread\n");
		array_push(thread_arr, (void *)(&ts));
	}

#ifndef _WIN32
	pthread_mutex_unlock(lock);
#endif
	return 0;
}
