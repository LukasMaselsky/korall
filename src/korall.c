#include "korall/korall.h"
#include "socket/socket.h"
#include "http/http_internal.h"
#include "arena/arena.h"
#include "lookup/lookup_tables.h"
#include "cJSON/cJSON.h"
#include "config/config.h"
#include "gui/gui.h"
#include "server/http/server_http.h"
#include "http/tls/tls.h"

// https://stackoverflow.com/questions/58885831/what-does-reaping-children-imply
// https://stackoverflow.com/questions/23401147/what-is-the-difference-between-struct-addrinfo-and-struct-sockaddr

static HTTPRoutes g_http_routes = { 0 };
static WebsocketRoutes g_ws_routes = { 0 };

static void http_routes_free(HTTPRoutes* routes) {
	if (routes == NULL || routes->routes == NULL) return;
	free(routes->routes);
}

static void http_routes_init(ServerConfig* config) {
	size_t capacity = sizeof(HTTPRoute) * config->max_http_routes;
	HTTPRoutes* routes = &g_http_routes;
	routes->routes = (HTTPRoute*)safe_calloc(capacity, sizeof(HTTPRoute));
	routes->capacity = capacity;
	routes->route_count = 0;
}

static void ws_routes_free(WebsocketRoutes* routes) {
	if (routes == NULL || routes->routes == NULL) {
		return;
	}
	free(routes->routes);
}

static void ws_routes_init(ServerConfig* config) {
	size_t capacity = sizeof(WebsocketRoute) * config->max_ws_routes;
	WebsocketRoutes* routes = &g_ws_routes;
	routes->routes = (WebsocketRoute*)safe_calloc(capacity, sizeof(WebsocketRoute));
	routes->capacity = capacity;
	routes->route_count = 0;
}

static routes_init() {
	ServerConfig* g_config = config_get();
	http_routes_init(g_config);
	ws_routes_init(g_config);
}

static routes_free(HTTPRoutes* http, WebsocketRoutes* ws) {
	http_routes_free(http);
	ws_routes_free(ws);
}

// 

void korall_http_routes_add(const char* path, const HTTPMethod method, void (* const callback)(const HTTPRequest*, HTTPResponse*)) {
	HTTPRoutes* routes = &g_http_routes;
	if (path == NULL) {
		log_msg(LOG_ERR, "failed to add route, path cannot be NULL\n");
		return;
	}
	if (callback == NULL) {
		log_msg(LOG_ERR, "failed to add route, callback cannot be NULL\n");
		return;
	}
	if (lookup_int_str(method, &http_method_lookup_table) == NULL) {
		log_msg(LOG_ERR, "failed to add route, method is not valid\n");
		return;
	}
	size_t count = routes->route_count;
	if (count >= routes->capacity) {
		log_msg(LOG_ERR, "failed to add route, maximum route count exceeded\n");
		return;
	}
	HTTPRoute route = { .path = path, .method = method, .callback = callback };
	memcpy(routes->routes + count, &route, sizeof(route));
	routes->route_count = count + 1;
}

void korall_ws_routes_add(const char* path, void (* const callback)(const WebsocketFrame*)) {
	WebsocketRoutes* routes = &g_ws_routes;
	if (path == NULL) {
		log_msg(LOG_ERR, "failed to add route, path cannot be NULL\n");
		return;
	}
	if (callback == NULL) {
		log_msg(LOG_ERR, "failed to add route, callback cannot be NULL\n");
		return;
	}

	size_t count = routes->route_count;
	if (count >= routes->capacity) {
		log_msg(LOG_ERR, "failed to add route, maximum route count exceeded\n");
		return;
	}
	WebsocketRoute route = { .path = path, .callback = callback };
	memcpy(routes->routes + count, &route, sizeof(route));
	routes->route_count = count + 1;
}

void korall_init(const char* config_path, const FILE* log_file) {
	
	logging_init(log_file);	
	config_init(config_path);
	routes_init();

}

void korall_run() {

	ServerConfig* g_config = config_get();

	// ssl
	SSL_CTX* ssl_ctx = NULL;
	if (g_config->secure) {
		openssl_init();

		ssl_ctx = openssl_create_server_ctx(g_config->resource_path);
		if (ssl_ctx == NULL) {
			log_msg(LOG_ERR, "failed to create server SSL_CTX\n");
		}
		else {
			log_msg(LOG_INFO, "created server SSL_CTX\n");
		}
	}

	// sockets
	
	int res = socket_init();
	if (res != 0) {
		log_msg(LOG_ERR, "socket initialisation failed, exiting\n");
		exit(EXIT_FAILURE);
	}

	SOCKET server_sock = init_listen_socket();
	

	ThreadState threads[MAX_THREADS] = { 0 };
	Array thread_arr = array_create_stack(threads, sizeof(ThreadState), 0, MAX_THREADS);
	
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

	ProcessDataArgs args = {
		.sock = 0,
		.http_routes = &g_http_routes,
		.ws_routes = &g_ws_routes,
		.thread_num = 0,
		.thread_arr = &thread_arr,
		.lock = &lock,
		.ssl = NULL,
	};

	SOCKET sock;

	while (true) {
		// todo: ensure 1 thread per connection (if doing http://localhost:3500 req then this happens when TLS fails)
		SSL* ssl = NULL;
		sock = process_incoming_connection(server_sock, ssl_ctx, &ssl);
		if (sock == INVALID_SOCKET) continue;
		
		sync_threads(&thread_arr, &lock);
	
		// spawn 1 thread for each connection

		// ! no mutex needed here since threads don't change array size
		
		if (array_full(&thread_arr)) { 
			log_msg(LOG_ERR, "not enough threads (max %d), could not process connection\n", MAX_THREADS);
			continue;
		}
		
		// make copy of args

		ProcessDataArgs* t_args = safe_calloc(1, sizeof(ProcessDataArgs));
		memcpy(t_args, &args, sizeof(*t_args));
		t_args->sock = sock;
		t_args->thread_num = thread_arr.size;
		t_args->ssl = ssl;

		create_data_process_thread(&thread_arr, t_args);
	}

	socket_close(server_sock);

	log_msg(LOG_INFO, "closed socket\n");

	socket_quit();

	// cleanup

	if (ssl_ctx != NULL) {
		SSL_CTX_free(ssl_ctx);
	}
	openssl_cleanup();
	routes_free(&g_http_routes, &g_ws_routes);
	config_free(g_config);

	return;
}
